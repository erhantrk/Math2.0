//
// Created by Erhan Türker on 10/31/25.
//

#pragma once

#include <memory>
#include <string>
#include <cmath>
#include "../../Node/inc/Node.hpp"

double factorial(double n);

std::shared_ptr<Node> simplify(const std::shared_ptr<Node>& node);