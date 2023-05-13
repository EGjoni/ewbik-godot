// bonestate.h
#pragma once

#include <string>
#include <unordered_map>
#include <vector>
#include "skeleton_state.h"
#include "target_state.h"
#include "constraint_state.h"
#include "transform_State.h"

class SkeletonState; // Forward declaration of SkeletonState

class BoneState {
public:
    BoneState(SkeletonState &skeletonState, std::string id, std::string transform_id, std::string parent_id, std::string target_id, std::string constraint_id, double stiffness);

    TransformState getTransform();
    TargetState getTarget();
    double getStiffness();
    BoneState getParent();
    BoneState getChild(const std::string &id);
    BoneState getChild(int index);
    void clearChildList();
    int getChildCount();
    int getTempChildCount();
    ConstraintState getConstraint();
    void prune();
    void setIndex(int index);
    void addChild(const std::string &id, int childIndex);
    void optimize();
    void validate();
    int getIndex();
    std::string getIdString();
    void setStiffness(double stiffness);

private:
    SkeletonState &skeletonState;
    std::string id;
    std::string parent_id;
    std::string transform_id;
    std::string target_id;
    std::string constraint_id;
    double stiffness;
    std::unordered_map<std::string, int> childMap;
    int index;
    int parentIdx;
    std::vector<int> childIndices;
    int transformIdx;
    int constraintIdx;
    int targetIdx;
};
