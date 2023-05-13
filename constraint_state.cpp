// constraint_state.cpp
#include "constraint_state.h"
#include "transform_state.h"

ConstraintState::ConstraintState(SkeletonState &skeletonState, std::string id, std::string forBone_id, std::string swingOrientationTransform_id, std::string twistOrientationTransform_id, Constraint directReference)
    : skeletonState(skeletonState), id(id), forBone_id(forBone_id), swingOrientationTransform_id(swingOrientationTransform_id), twistOrientationTransform_id(twistOrientationTransform_id), directReference(directReference), index(0), swingTransform_idx(-1), twistTransform_idx(-1) {}

void ConstraintState::prune() {
    if (getTwistTransform() != nullptr) {
        getTwistTransform()->prune();
    }
    getSwingTransform()->prune();
    skeletonState.constraintMap.erase(this->id);
    skeletonState.constraintsList[this->index] = nullptr;
}

TransformState *ConstraintState::getSwingTransform() {
    return skeletonState.transforms[this->swingTransform_idx];
}

TransformState *ConstraintState::getTwistTransform() {
    if (this->twistTransform_idx == -1) {
        return nullptr;
    }
    return skeletonState.transforms[this->twistTransform_idx];
}

void ConstraintState::setIndex(int index) {
    this->index = index;
}

int ConstraintState::getIndex() {
    return this->index;
}

void ConstraintState::optimize() {
    if (this->twistOrientationTransform_id != "") {
        TransformState *twistTransform = skeletonState.transformMap[this->twistOrientationTransform_id];
        this->twistTransform_idx = twistTransform->getIndex();
    }
    TransformState *swingTransform = skeletonState.transformMap[this->swingOrientationTransform_id];
    this->swingTransform_idx = swingTransform->getIndex();
}

void ConstraintState::validate() {
    if (skeletonState.assumeValid) {
        return;
    }

    BoneState *forBone = skeletonState.boneMap[this->forBone_id];
    if (forBone == nullptr) {
        throw std::runtime_error("Constraint '" + this->id + "' claims to constrain bone '" + forBone_id + "', but no such bone has been registered with this SkeletonState");
    }
    if (this->swingOrientationTransform_id == "") {
        throw std::runtime_error("Constraint with id '" + this->id + "' claims to have no swing transform, but this transform is required. "
                                  "You may provide an identity transform if you wish to indicate that the constraint's swing space is equivalent to the parent bone's default space");
    }
    TransformState *constraintSwing = skeletonState.transformMap[this->swingOrientationTransform_id];
    if (constraintSwing == nullptr) {
        throw std::runtime_error("Constraint with id '" + this->id + "' claims to have a swingOrientationTransform with id'" + this->swingOrientationTransform_id + "', but no such transform has been registered with this SkeletonState'");
    }
    if (this->twistOrientationTransform_id != "") {
        TransformState *constraintTwist = skeletonState.transformMap[this->twistOrientationTransform_id];
        if (constraintTwist == nullptr) {
            throw std::runtime_error("Constraint with id '" + this->id + "' claims to have a twist transform with id'" + this->twistOrientationTransform_id + "', but no such transform has been registered with this SkeletonState'");
        }
    }
}

std::string ConstraintState::getIdString() {
    return this->id;
}

Constraint ConstraintState::getDirectReference() {
    return this->directReference;
}
