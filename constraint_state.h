#ifndef CONSTRAINT_STATE_H
#define CONSTRAINT_STATE_H

#include "core/string/ustring.h"
#include "core/error/error_macros.h"
#include "skeleton_state.h"
#include "transform_state.h"
#include "constraint.h"
#include "bone_state.h"

using String = Godot::String;
class SkeletonState;
class ConstraintState {
public:
    ConstraintState(SkeletonState &skeletonState, String id, String forBone_id, String swingOrientationTransform_id, String twistOrientationTransform_id, Constraint directReference);

    void prune();
    TransformState *getSwingTransform();
    TransformState *getTwistTransform();
    void setIndex(int index);
    int getIndex();
    void optimize();
    void validate();
    String getIdString();
    Constraint getDirectReference();

private:
    SkeletonState &skeletonState;
    String id;
    String forBone_id;
    String swingOrientationTransform_id;
    String twistOrientationTransform_id;
    Constraint directReference;
    int index;
    int swingTransform_idx;
    int twistTransform_idx;
};

#endif // CONSTRAINT_STATE_H
