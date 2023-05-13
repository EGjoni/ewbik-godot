#ifndef SKELETON_STATE_H
#define SKELETON_STATE_H

#include "core/ustring.h"
#include "core/map.h"
#include "core/vector.h"
#include "core/reference.h"
#include "core/math/transform.h"
#include "bone_state.h"
#include "constraint_state.h"
#include "target_state.h"
#include "transform_state.h"

class BoneState;
class TransformState;
class TargetState;
class ConstraintState;

class SkeletonState : public Reference {
    GDCLASS(SkeletonState, Reference)

public:
    SkeletonState(bool p_assume_valid = false);

    Ref<BoneState> add_bone(const String &p_id, const String &p_transform_id, const String &p_parent_id, const String &p_constraint_id, real_t p_stiffness, const String &p_target_id);
    Ref<ConstraintState> add_constraint(const String &p_id, const String &p_for_bone_id, const String &p_swing_orientation_transform_id, const String &p_twist_orientation_transform_id, Ref<ConstraintState> p_direct_reference);
    Ref<TargetState> add_target(const String &p_id, const String &p_transform_id, const String &p_for_bone_id, const Vector<double> &p_priorities, real_t p_depth_falloff, real_t p_weight);
    void add_transform(const String &p_id, const Vector3 &p_translation, const Basis &p_rotation, const Vector3 &p_scale, const String &p_parent_id, const Variant &p_direct_reference);

    // Other public methods.

friend class BoneState;
friend class ConstraintState;
friend class TargetState;
friend class TransformState;

protected:
    static void _bind_methods();

private:
    bool assumeValid;
};

#endif // SKELETON_STATE_H
