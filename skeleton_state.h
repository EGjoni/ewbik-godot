#ifndef SKELETON_STATE_H
#define SKELETON_STATE_H

#include "core/string/ustring.h"
#include "core/templates/vector.h"
#include "core/templates/hash_map.h"
#include "bone_state.h"
#include "transform_state.h"
#include "constraint_state.h"
#include "target_state.h"

class SkeletonState {
public:
    SkeletonState();
    SkeletonState(bool assume_valid);

    BoneState *get_bone_state(int index);
    int get_bone_count();
    TransformState *get_transform_state(int index);
    int get_transform_count();
    ConstraintState *get_constraint_state(int index);
    int get_constraint_count();
    TargetState *get_target_state(int index);
    int get_target_count();

    void validate();
    void prune();
    BoneState *add_bone(String id, String transform_id, String parent_id, String constraint_id, double stiffness, String target_id);
    ConstraintState *add_constraint(String id, String for_bone_id, String swing_orientation_transform_id, String twist_orientation_transform_id, Constraint *direct_reference);
    TransformState *add_transform(String id, Transform *transform, String parent_id, bool invert);
    TargetState *add_target(String id, String transform_id, String target_parent_id, Target *direct_reference);
    void optimize();

friend class BoneState;
friend class ConstraintState;
friend class TargetState;
friend class TransformState;

private:
    bool assume_valid;
    BoneState *root_bone_state = nullptr;
    Vector<BoneState *> bones;
    Vector<TransformState *> transforms;
    Vector<ConstraintState *> constraints;
    Vector<TargetState *> targets;
    HashMap<String, BoneState *> bone_map;
    HashMap<String, TransformState *> transform_map;
    HashMap<String, ConstraintState *> constraint_map;
    HashMap<String, TargetState *> target_map;
};

#endif // SKELETON_STATE_H
