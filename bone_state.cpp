#include "core/string_name.h"
#include "core/dictionary.h"
#include "core/vector.h"

BoneState::BoneState(String id, String transform_id, String parent_id, String target_id, String constraint_id, real_t stiffness)
    : id(id), parent_id(parent_id), transform_id(transform_id), target_id(target_id), constraint_id(constraint_id), stiffness(stiffness), parentIdx(-1), transformIdx(-1), constraintIdx(-1), targetIdx(-1) {}

TransformState getTransform()
{
    return transforms[transformIdx];
}

TargetState getTarget()
{
    if (targetIdx == -1)
        return nullptr;
    else
        return targets[targetIdx];
}

real_t getStiffness()
{
    return stiffness;
}

BoneState getParent()
{
    if (parentIdx >= 0)
        return bones[parentIdx];
    return nullptr;
}

BoneState getChild(String id)
{
    return bones[childMap[id]];
}

BoneState getChild(int index)
{
    return bones[childIndices[index]];
}

void clearChildList()
{
    childMap.clear();
    childIndices.clear();
}

int getChildCount()
{
    return childIndices.size();
}

int getTempChildCount()
{
    return childMap.size();
}

ConstraintState getConstraint()
{
    if (constraintIdx >= 0)
        return constraints[constraintIdx];
    return nullptr;
}

void prune()
{
    bonesList[index] = nullptr;
    boneMap.erase(id);
    getTransform().prune();
    if (getConstraint() != nullptr)
        getConstraint().prune();
    if (parent_id != "")
    {
        boneMap[parent_id].childMap.erase(id);
    }
    if (rootBoneState == this)
        rootBoneState = nullptr;
}

void setIndex(int index)
{
    this->index = index;
    if (parent_id != "")
    {
        BoneState &parentBone = boneMap[parent_id];
        parentBone.addChild(id, index);
    }
}

void addChild(String id, int childIndex)
{
    childMap[id] = childIndex;
}

void optimize()
{
    Vector<int> tempChildren(childMap.size());
    int i = 0;
    for (const auto &entry : childMap)
        tempChildren[i++] = entry.second;
    childIndices = tempChildren;
    if (parent_id != "")
        parentIdx = boneMap[parent_id].index;
    transformIdx = transformMap[transform_id].getIndex();
    if (constraint_id != "")
        constraintIdx = constraintMap[constraint_id].getIndex();
    if (target_id != "")
        targetIdx = targetMap[target_id].getIndex();
}

void validate()
{
    if (skeletonState.assumeValid)
        return;

    if (transformMap.find(transform_id) == transformMap.end()) // check that the bone has a transform
        throw std::runtime_error("Bone '" + id.utf8().get_data() + "' references transform with id '" + transform_id.utf8().get_data() + "', but '" + transform_id.utf8().get_data() + "' has not been registered with the SkeletonState.");

    if (!parent_id.empty())
    {                                                 // if this isn't a root bone, ensure the following
        if (boneMap.find(parent_id) == boneMap.end()) // check that the bone listed as its parent has been registered.
            throw std::runtime_error("Bone '" + id.utf8().get_data() + "' references parent bone with id '" + parent_id.utf8().get_data() + "', but '" + parent_id.utf8().get_data() + "' has not been registered with the SkeletonState.");
        TransformState parentBonesTransform = transformMap[parent_id];
        TransformState transformsParent = transformMap[parentBonesTransform.parent_id];

        if (parentBonesTransform != transformsParent)
        { // check that the parent transform of this bones transform is the same as the transform of the bone's parent
            throw std::runtime_error("Bone '" + id.utf8().get_data() + "' has listed bone with id '" + parent_id.utf8().get_data() + "' as its parent, which has a transform_id of '" + parentBonesTransform.transform_id.utf8().get_data() +
                                     "' but the parent transform of this bone's transform is listed as " + transformsParent.parent_id.utf8().get_data() + "'. A bone's transform must have the parent bone's transform as its parent");
        }

        // avoid grandfather paradoxes
        BoneState *ancestor = &boneMap[parent_id];
        while (ancestor != nullptr)
        {
            if (ancestor == this)
            {
                throw std::runtime_error("Bone '" + id.utf8().get_data() + "' is listed as being both a descendant and an ancestor of itself.");
            }
            if (ancestor->parent_id.empty())
            {
                ancestor = nullptr;
            }
            else
            {
                BoneState *curr = ancestor;
                ancestor = &boneMap[curr->parent_id];
                if (ancestor == nullptr)
                {
                    throw std::runtime_error("bone with id `" + curr->id.utf8().get_data() + "` lists its parent bone as having id `" + curr->parent_id.utf8().get_data() + "', but no such bone has been registered with the SkeletonState");
                }
            }
        }
    }
    else
    {
        if (!constraint_id.empty())
        {
            throw std::runtime_error("Bone '" + id.utf8().get_data() + "' has been determined to be a root bone. However, root bones may not be constrained." + "If you wish to constrain the root bone anyway, please insert a fake unconstrained root bone prior to this bone. Give that bone's transform values equal to this bone's, and set this " + "bone's transforms to identity.");
        }
    }

    if (!constraint_id.empty())
    {                                                                 // if this bone has a constraint, ensure the following:
        if (constraintMap.find(constraint_id) == constraintMap.end()) // check that the constraint has been registered
            throw std::runtime_error("Bone '" + id.utf8().get_data() + "' claims to be constrained by '" + constraint_id.utf8().get_data() + "', but no such constraint has been registered with this SkeletonState");

        ConstraintState constraint = constraintMap[constraint_id];
        if (constraint.forBone_id != id)
        {
            throw std::runtime_error("Bone '" + id.utf8().get_data() + "' claims to be constrained by '" + constraint.id.utf8().get_data() + "', but constraint of id '" + constraint.id.utf8().get_data() + "' claims to be constraining bone with id '" + constraint.forBone_id.utf8().get_data() + "'");
        }
    }
}

int getIndex()
{
    return index;
}

String getIdString()
{
    return id;
}

void setStiffness(real_t stiffness)
{
    this->stiffness = stiffness;
}

BoneState::BoneState(SkeletonState &skeletonState, String id, String transform_id, String parent_id, String target_id, String constraint_id, real_t stiffness)
    : skeletonState(skeletonState), id(id), parent_id(parent_id), transform_id(transform_id), target_id(target_id), constraint_id(constraint_id), stiffness(stiffness), parentIdx(-1), transformIdx(-1), constraintIdx(-1), targetIdx(-1) {}