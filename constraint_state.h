// constraint_state.h
#pragma once

#include <string>
#include "skeleton_state.h" // Include the SkeletonState header

class SkeletonState; // Forward declaration of SkeletonState

class ConstraintState {
public:
    ConstraintState(SkeletonState &skeletonState, std::string id, std::string forBone_id, std::string swingOrientationTransform_id, std::string twistOrientationTransform_id, Constraint directReference);

    void prune();
    TransformState getSwingTransform();
    TransformState getTwistTransform();
    int getIndex();
    std::string getIdString();
    Constraint getDirectReference();

private:
    void setIndex(int index);
    void optimize();
    void validate();

    SkeletonState &skeletonState;
    std::string id;
    std::string forBone_id;
    std::string swingOrientationTransform_id;
    std::string twistOrientationTransform_id;
    Constraint directReference;
    int index;
    int swingTransform_idx;
    int twistTransform_idx;
};
