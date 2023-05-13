#include "target_state.h"
#include "skeleton_state.h"
#include "transform_state.h"
#include <stdexcept>
#include <algorithm>

TargetState::TargetState(const std::string &id, const std::string &transform_id, const std::string &forBone_id, std::vector<double> priorities, double depthFalloff, double weight, SkeletonState &skeletonState)
    : skeletonState(skeletonState) {
    init(id, transform_id, forBone_id, priorities, depthFalloff, weight);
}

void TargetState::init(const std::string &id, const std::string &transform_id, const std::string &forBone_id, const std::vector<double> &priorities, double depthFalloff, double weight) {
    this->id = id;
    this->forBone_id = forBone_id;
    this->transform_id = transform_id;
    this->modeCode = 0;
    this->priorities = priorities;
    this->depthFalloff = depthFalloff;
    this->weight = weight;

    bool xDir = this->priorities[0] > 0;
    bool yDir = this->priorities[1] > 0;
    bool zDir = this->priorities[2] > 0;

    modeCode = 0;
    if (xDir) modeCode += 1;
    if (yDir) modeCode += 2;
    if (zDir) modeCode += 4;
}

void TargetState::setIndex(int index) {
    this->index = index;
}

void TargetState::optimize() {
    this->forBoneIdx = skeletonState.getBoneIndex(this->forBone_id);
    this->transformIdx = skeletonState.getTransformIndex(this->transform_id);
}

TransformState* TargetState::getTransform() {
    return skeletonState.getTransformAtIndex(this->transformIdx);
}

void TargetState::validate() {
    TransformState* transform = skeletonState.getTransformByID(this->transform_id);
    if (transform == nullptr) {
        throw std::runtime_error("Target with id '" + this->id + "' lists its transform as having id '" + this->transform_id + "', but no such transform has been registered with this StateSkeleton");
    }
    if (!transform->getParentID().empty()) {
        throw std::runtime_error("Target with id '" + this->id + "' lists its transform as having a parent transform. However, target transforms are not allowed to have a parent, as they must be given in the space of the skeleton transform. Please provide a transform object that has been converted into skeleton space and has no parent.");
    }
}

std::string TargetState::getIdString() {
    return this->id;
}

uint8_t TargetState::getModeCode() {
    return this->modeCode;
}

double TargetState::getDepthFalloff() {
    return this->depthFalloff;
}

double TargetState::getWeight() {
    return this->weight;
}

int TargetState::getIndex() {
    return this->index;
}

double TargetState::getPriority(int basisDirection) {
    return priorities[basisDirection / 2];
}

double TargetState::getMaxPriority() {
    double maxPinWeight = 0;
    if ((modeCode & AbstractIKPin::XDir) != 0) {
        maxPinWeight = std::max(maxPinWeight, this->getPriority(AbstractIKPin::XDir));
    }
    if ((modeCode & AbstractIKPin::YDir) != 0) {
        maxPinWeight = std::max(maxPinWeight, this->getPriority(AbstractIKPin::YDir));
    }
    if ((modeCode & AbstractIKPin::ZDir) != 0) {
        maxPinWeight = std::max(maxPinWeight, this->getPriority(AbstractIKPin::ZDir));
    }
    return maxPinWeight;
}

