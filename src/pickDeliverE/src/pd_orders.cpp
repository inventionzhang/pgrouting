/*PGR-GNU*****************************************************************

FILE: solution.cpp

Copyright (c) 2015 pgRouting developers
Mail: project@pgrouting.org

------

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.

 ********************************************************************PGR-GNU*/

#include "vrp/pd_orders.h"

#include <vector>
#include <memory>
#include <utility>

#include "vrp/order.h"
#include "vrp/node.h"
#include "vrp/tw_node.h"
#include "vrp/vehicle_node.h"
#include "vrp/pgr_pickDeliver.h"

namespace pgrouting {
namespace vrp {


PD_Orders::PD_Orders(
        const std::vector<PickDeliveryOrders_t> &pd_orders
        ) {
    build_orders(pd_orders);
}


void
PD_Orders::build_orders(
        const std::vector<PickDeliveryOrders_t> &pd_orders
        ) {
    OID order_id(0);
    for (const auto order : pd_orders) {
        ENTERING();
        /*
         * SAMPLE CORRECT INFORMATION
         *
         * id | demand | pick_x | pick_y | pick_open_t | pick_close_t | pick_service_t | deliver_x | deliver_y | deliver_open_t | deliver_open_t | deliver_close_t | deliver_service_t
         * 1  | 10     |   35   |   69   |   448       |   505        |    90          |    45     |   68      |    912         |   967          |    90           |    35
         */

        /*
         * Creating the pickup & delivery nodes
         */
        std::unique_ptr<Base_node> b_pick(new pickdeliver::Node(
                problem->node_id(),
                order.pick_node_id,
                order.pick_x,
                order.pick_y));
        msg.log <<  order.id << ": " << problem->node_id()
            << "," << order.pick_node_id << "\n";
        Vehicle_node pickup(
                {problem->node_id()++, order, Tw_node::NodeType::kPickup});

#if 0
        pgassert(!problem->m_cost_matrix.empty());
        msg.log << b_pick->idx() << ",";
        msg.log << b_pick->id() << "\n";
        msg.log << pickup.idx() << ",";
        msg.log << pickup.id() << "\n";
#endif

        std::unique_ptr<Base_node> b_drop(new pickdeliver::Node(
                problem->node_id(),
                order.deliver_node_id,
                order.deliver_x,
                order.deliver_y));
        Vehicle_node delivery(
                {problem->node_id()++, order, Tw_node::NodeType::kDelivery});

#if 0
        msg.log << "pick " << pickup << "\n";
        msg.log << "drop " << delivery << "\n";
        msg.log << "distance " << pickup.distance(delivery) << "\n";
        msg.log << ".............\n";
        msg.log << "pick " << *b_pick << "\n";
        msg.log << "drop " << *b_drop << "\n";
        pickdeliver::Node p = *dynamic_cast<pickdeliver::Node*>(b_pick.get());
        pickdeliver::Node d = *dynamic_cast<pickdeliver::Node*>(b_drop.get());
        msg.log << "distance " << p.distance(d) << "\n";
        msg.log << "distance " << p.distance(b_drop.get()) << "\n";
        msg.log << "distance " << b_pick->distance(*b_drop.get()) << "\n";

        pgassertwm(
                pickup.distance(delivery) == b_pick->distance(*b_drop.get()),
                msg.get_log().c_str());

        pickup.set_Did(delivery.idx());
        delivery.set_Pid(pickup.idx());
#endif

        problem->add_base_node(std::move(b_pick));
        problem->add_base_node(std::move(b_drop));
        problem->add_node(pickup);
        problem->add_node(delivery);

        pgassert(problem->nodesOK());



        /*
         * add into an order
         */
        m_orders.push_back(
                Order(order_id++, order.id,
                    pickup,
                    delivery));
    }  //  for (creating orders)

#if 0
    for (auto &o : m_orders) {
        o.setCompatibles();
    }
#endif
    EXITING();
}

bool
PD_Orders::is_valid(double speed) const {
    for (const auto &o : m_orders) {
        if (!o.is_valid(speed)) {
            return false;
        }
        pgassert(o.pickup().is_pickup());
        pgassert(o.delivery().is_delivery());
        /* P -> D */
        pgassert(o.delivery().is_compatible_IJ(o.pickup(), speed));
    }
    return true;
}

Order&
PD_Orders::operator[](OID i) {
    pgassert(i < m_orders.size());
    return m_orders[i];
}

const Order&
PD_Orders::operator[](OID i) const {
    pgassert(i < m_orders.size());
    return m_orders[i];
}

void
PD_Orders::set_compatibles(double speed) {
    for (auto &I : m_orders) {
        for (const auto J : m_orders) {
            I.set_compatibles(J, speed);
        }
    }
}

size_t
PD_Orders::find_best_J(
        Identifiers<size_t> &within_this_set) const {
    pgassert(!within_this_set.empty());
    auto best_order = within_this_set.front();
    size_t max_size = 0;


    for (auto o : within_this_set) {
        auto size_J =  m_orders[o].subsetJ(within_this_set).size();
        if (max_size < size_J) {
            max_size = size_J;
            best_order = o;
        }
    }
    return best_order;
}


size_t
PD_Orders::find_best_I(
        Identifiers<size_t> &within_this_set) const {
    pgassert(!within_this_set.empty());
    auto best_order = within_this_set.front();
    size_t max_size = 0;


    for (auto o : within_this_set) {
        auto size_I =  m_orders[o].subsetI(within_this_set).size();
        if (max_size < size_I) {
            max_size = size_I;
            best_order = o;
        }
    }
    return best_order;
}


}  //  namespace vrp
}  //  namespace pgrouting
