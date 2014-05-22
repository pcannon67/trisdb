/* 
 * File:   QueryPlanner.cpp
 * Author: tiepologian <tiepolo.gian@gmail.com>
 * 
 * Created on 21 maggio 2014, 21.52
 */

#include "QueryPlanner.h"
#include "TrisDb.h"

QueryPlanner::QueryPlanner(TrisDb* parent) {
    this->_parent = parent;
}

QueryPlanner::QueryPlanner(const QueryPlanner& orig) {
    //
}

QueryPlanner::~QueryPlanner() {
    //
}

Utils::ResultVector QueryPlanner::execute(QueryParser::Query q) {
    Utils::ResultVector result;
    if (q.command == "CREATE") {
        this->_parent->create(std::get<0>(q.parameters), std::get<1>(q.parameters), std::get<2>(q.parameters));
    } else if (q.command == "GET" || q.command == "GETS" || q.command == "GETP" || q.command == "GETO") {
        std::deque<int> plan = getQueryPlan(q);
        
        //std::cout << "PARAMS:" << std::endl;
        //std::cout << std::get<0>(q.parameters) << "-" << std::get<1>(q.parameters) << "-" << std::get<2>(q.parameters) << std::endl;
        
        //std::cout << "QUERY PLAN:" << std::endl;
        //for (auto i : plan) std::cout << i << std::endl;
        
        std::string args[3];
        std::tie(args[0], args[1], args[2]) = q.parameters;
        
        Utils::ResultVector tmp1 = this->_parent->get(plan[0], args[plan[0]]);
        //std::cout << "Checking " << plan[0] << " for value " << args[plan[0]] << std::endl;
        //std::cout << "TMP:" << std::endl;
        for (Utils::ResultVector::iterator it = tmp1.begin(); it != tmp1.end(); ++it) {
            //std::cout << std::get<0>(*it) << "-" << std::get<1>(*it) << "-" << std::get<2>(*it) << std::endl;
            if(isEqual(std::get<0>(*it), std::get<0>(q.parameters)) && isEqual(std::get<1>(*it), std::get<1>(q.parameters)) && isEqual(std::get<2>(*it), std::get<2>(q.parameters))) {
                result.push_back(*it);
            }           
        }
    } else if(q.command == "QUIT") {
        exit(0);
    }
    return result;
}

std::deque<int> QueryPlanner::getQueryPlan(QueryParser::Query q) {
    std::deque<int> plan;
    Utils::record r = q.parameters;

    if (std::get<0>(r) == Utils::kQueryWildcard) plan.push_back(0);
    else plan.push_front(0);

    if (std::get<1>(r) == Utils::kQueryWildcard) plan.push_back(1);
    else plan.push_front(1);

    if (std::get<2>(r) == Utils::kQueryWildcard) plan.push_back(2);
    else plan.push_front(2);

    return plan;
}

Utils::ResultVector QueryPlanner::getIntersection(Utils::ResultVector &a, Utils::ResultVector &b) {
    Utils::ResultVector res;
    for (Utils::ResultVector::iterator i = a.begin(); i != a.end(); ++i) {
        if (std::find(b.begin(), b.end(), *i) != b.end()) {
            res.push_back(*i);
        }
    }
}

bool QueryPlanner::isEqual(std::string s1, std::string s2) {
    if((s2 == Utils::kQueryWildcard) || (s1 == s2)) return true;
    else return false;
}