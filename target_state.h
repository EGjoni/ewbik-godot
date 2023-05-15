#ifndef TARGET_STATE_H
#define TARGET_STATE_H

#include "core/string/ustring.h"
#include "core/templates/vector.h"
#include "skeleton_state.h"
#include "transform_state.h"
class SkeletonState;
class TargetState {
public:
    TargetState(const String &p_id, const String &p_transform_id, const String &p_for_bone_id, Vector<float> p_priorities, float p_depth_falloff, float p_weight, SkeletonState &p_skeleton_state);

    void set_index(int p_index);
    void optimize();
    TransformState* get_transform();
    void validate();
    String get_id_string();
    uint8_t get_mode_code();
    float get_depth_falloff();
    float get_weight();
    int get_index();
    float get_priority(int p_basis_direction);
    float get_max_priority();

private:
    void init(const String &p_id, const String &p_transform_id, const String &p_for_bone_id, const Vector<float> &p_priorities, float p_depth_falloff, float p_weight);

    String id;
    String for_bone_id;
    String transform_id;
    uint8_t mode_code;
    Vector<float> priorities;
    float depth_falloff;
    float weight;
    int index;

    int for_bone_idx;
    int transform_idx;
    SkeletonState &skeleton_state;
};

#endif // TARGET_STATE_H
