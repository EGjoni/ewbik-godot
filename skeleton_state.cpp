#include "skeleton_state.h"

SkeletonState::SkeletonState()
{
    // Initialize instance variables
    assume_valid = false;
}

SkeletonState::SkeletonState(bool assume_valid)
{
    this->assume_valid = assume_valid;
}

BoneState *SkeletonState::get_bone_state(int index)
{
    return bones[index];
}

int SkeletonState::get_bone_count()
{
    return bones.size();
}

TransformState *SkeletonState::get_transform_state(int index)
{
    return transforms[index];
}

int SkeletonState::get_transform_count()
{
    return transforms.size();
}

ConstraintState *SkeletonState::get_constraint_state(int index)
{
    return constraints[index];
}

int SkeletonState::get_constraint_count()
{
    return constraints.size();
}

TargetState *SkeletonState::get_target_state(int index)
{
    return targets[index];
}

int SkeletonState::get_target_count()
{
    return targets.size();
}

void SkeletonState::validate() {
    for (auto& kv : bone_map) {
        BoneState* bs = kv.second;
        if (bs->parent_id.empty()) {
            if (root_bone_state != nullptr) {
                throw std::runtime_error("A skeleton may only have 1 root bone, you attempted to initialize bone of id `" + bs->id + "` as an implicit root (no parent bone), when bone with id '" + root_bone_state->id + "' is already determined as being the implicit root");
            }
            root_bone_state = bs;
        }
    }
    for (auto& kv : bone_map) {
        BoneState* bs = kv.second;
        bs->validate();
        bones.push_back(bs);
    }
    for (auto& kv : transform_map) {
        TransformState* ts = kv.second;
        ts->validate();
        transforms.push_back(ts);
    }
    for (auto& kv : target_map) {
        TargetState* ts = kv.second;
        ts->validate();
        targets.push_back(ts);
    }
    for (auto& kv : constraint_map) {
        ConstraintState* cs = kv.second;
        cs->validate();
        constraints.push_back(cs);
    }
    optimize();
    prune();
}


void SkeletonState::prune()
{
    // The implementation of the prune method, translated from Java
    std::vector<BoneState *> leaf_bones;
    for (auto &bone : bones)
    {
        if (bone->get_child_count() == 0)
        {
            leaf_bones.push_back(bone);
        }
    }
    for (auto &leaf : leaf_bones)
    {
        BoneState *current_leaf = leaf;
        while (current_leaf->target_id.empty())
        {
            current_leaf->prune();
            current_leaf = current_leaf->get_parent();
        }
    }
    optimize();
}

BoneState *SkeletonState::add_bone(String id, String transform_id, String parent_id, String constraint_id, double stiffness, String target_id)
{
    // The implementation of the add_bone method, translated from Java
    BoneState *result = new BoneState(id, transform_id, parent_id, target_id, constraint_id, stiffness);
    bone_map[id] = result;
    bones.push_back(result);
    return result;
}

ConstraintState *SkeletonState::add_constraint(String id, String for_bone_id, String swing_orientation_transform_id, String twist_orientation_transform_id, Constraint *direct_reference)
{
    // The implementation of the add_constraint method, translated from Java
    ConstraintState *con = new ConstraintState(id, for_bone_id, swing_orientation_transform_id, twist_orientation_transform_id, direct_reference);
    constraint_map[id] = con;
    constraints.push_back(con);
    return con;
}

TransformState *SkeletonState::add_transform(String id, Transform *transform, String parent_id, bool invert)
{
    // The implementation of the add_transform method, translated from Java
    TransformState *result = new TransformState(id, transform, parent_id, invert);
    transform_map[id] = result;
    transforms.push_back(result);
    return result;
}

TargetState *SkeletonState::add_target(String id, String transform_id, String target_parent_id, Target *direct_reference)
{
    // The implementation of the add_target method, translated from Java
    TargetState *result = new TargetState(id, transform_id, target_parent_id, direct_reference);
    target_map[id] = result;
    targets.push_back(result);
    return result;
}

void SkeletonState::optimize()
{
    // The implementation of the optimize method, translated from Java
    std::vector<BoneState *> new_bones;
    std::vector<TransformState *> new_transforms;
    std::vector<ConstraintState *> new_constraints;
    std::vector<TargetState *> new_targets;

    for (auto &bone : bones)
    {
        if (bone->get_child_count() > 0 || !bone->target_id.empty())
        {
            new_bones.push_back(bone);
        }
    }
    for (auto &transform : transforms)
    {
        if (transform->child_count > 0 || transform->parent_id.empty())
        {
            new_transforms.push_back(transform);
        }
    }
    for (auto &constraint : constraints)
    {
        if (constraint->for_bone_id.empty())
        {
            new_constraints.push_back(constraint);
        }
    }
    for (auto &target : targets)
    {
        if (!target->transform_id.empty())
        {
            new_targets.push_back(target);
        }
    }

    bones = new_bones;
    transforms = new_transforms;
    constraints = new_constraints;
    targets = new_targets;
}