#pragma once

#include "core/reference.h"
#include "core/string.h"
#include "core/vector.h"
#include "core/dictionary.h"

class TransformState;
class TargetState;
class ConstraintState;
class SkeletonState;

class BoneState {
public:
    BoneState(String id, String transform_id, String parent_id, String target_id, String constraint_id, real_t stiffness);
    TransformState getTransform();
    TargetState getTarget();
    real_t getStiffness();
    BoneState getParent();
    BoneState getChild(String id);
    BoneState getChild(int index);
    void clearChildList();
    int getChildCount();
    int getTempChildCount();
    ConstraintState getConstraint();
    void prune();
    void setIndex(int index);
    void addChild(String id, int childIndex);
    void optimize();
    void validate();
    int getIndex();
    String getIdString();
    void setStiffness(real_t stiffness);
    BoneState(SkeletonState &skeletonState, String id, String transform_id, String parent_id, String target_id, String constraint_id, real_t stiffness);

private:
    SkeletonState &skeletonState;
    String id;
    String parent_id;
    String transform_id;
    String target_id;
    String constraint_id;
    real_t stiffness;
    int parentIdx;
    int transformIdx;
    int constraintIdx;
    int targetIdx;
    int index;
    Dictionary childMap;
    Vector<int> childIndices;
};
