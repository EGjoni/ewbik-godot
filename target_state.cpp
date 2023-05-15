#include "target_state.h"
#include "skeleton_state.h"
#include "transform_state.h"
#include <stdexcept>
#include <algorithm>
#include "core/error/error_macros.h"

TargetState::TargetState(const String &p_id, const String &p_transform_id, const String &p_for_bone_id, Vector<float> p_priorities, float p_depth_falloff, float p_weight, SkeletonState &p_skeleton_state)
    : skeleton_state(p_skeleton_state)
{
    init(p_id, p_transform_id, p_for_bone_id, p_priorities, p_depth_falloff, p_weight);
}

void TargetState::init(const String &p_id, const String &p_transform_id, const String &p_for_bone_id, const Vector<float> &p_priorities, float p_depth_falloff, float p_weight)
{
    id = p_id;
    for_bone_id = p_for_bone_id;
    transform_id = p_transform_id;
    mode_code = 0;
    priorities = p_priorities;
    depth_falloff = p_depth_falloff;
    weight = p_weight;

    bool x_dir = priorities[0] > 0;
    bool y_dir = priorities[1] > 0;
    bool z_dir = priorities[2] > 0;

    mode_code = 0;
    if (x_dir)
        mode_code += 1;
    if (y_dir)
        mode_code += 2;
    if (z_dir)
        mode_code += 4;
}

void TargetState::set_index(int p_index)
{
    index = p_index;
}

void TargetState::optimize()
{
    for_bone_idx = skeleton_state.get_bone_index(for_bone_id);
    transform_idx = skeleton_state.get_transform_index(transform_id);
}

TransformState *TargetState::get_transform()
{
    return skeleton_state.get_transform_at_index(transform_idx);
}

void TargetState::validate()
{
    TransformState *transform = skeleton_state.get_transform_by_id(transform_id);
    if (transform == nullptr)
    {
        ERR_FAIL_MSG("Target with id '" + id + "' lists its transform as having id '" + transform_id + "', but no such transform has been registered with this StateSkeleton.");
    }
    if (!transform->get_parent_id().empty())
    {
        ERR_FAIL_MSG("Target with id '" + id + "' lists its transform as having a parent transform. However, target transforms are not allowed to have a parent, as they must be given in the space of the skeleton transform. Please provide a transform object that has been converted into skeleton space and has no parent.");
    }
}

String TargetState::get_id_string()
{
    return id;
}

uint8_t TargetState::get_mode_code()
{
    return mode_code;
}

float TargetState::get_depth_falloff()
{
    return depth_falloff;
}

float TargetState::get_weight()
{
    return weight;
}

int TargetState::get_index()
{
    return index;
}

float TargetState::get_priority(int p_basis_direction)
{
    return priorities[p_basis_direction / 2];
}

float TargetState::get_max_priority()
{
    float max_pin_weight = 0;
    if ((mode_code & AbstractIKPin::XDir) != 0)
    {
        max_pin_weight = MAX(max_pin_weight, get_priority(AbstractIKPin::XDir));
    }
    if ((mode_code & AbstractIKPin::YDir) != 0)
    {
        max_pin_weight = MAX(max_pin_weight, get_priority(AbstractIKPin::YDir));
    }
    if ((mode_code & AbstractIKPin::ZDir) != 0)
    {
        max_pin_weight = MAX(max_pin_weight, get_priority(AbstractIKPin::ZDir));
    }
    return max_pin_weight;
}
