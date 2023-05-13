#pragma once

#include <string>
#include <vector>
#include <stdexcept>
#include "skeleton_state.h"

class TransformState {
public:
    TransformState(SkeletonState &skeletonState, std::string id, std::vector<double> translation, std::vector<double> rotation, std::vector<double> scale, std::string parent_id, void* directReference);

    void update(std::vector<double> translation, std::vector<double> rotation, std::vector<double> scale);
    void prune();
    int getIndex();
    int getParentIndex();
    TransformState* getParentTransform();

private:
    SkeletonState &skeletonState;
    std::string id;
    std::vector<double> translation;
    std::vector<double> rotation;
    std::vector<double> scale;
    std::string parent_id;
    void* directReference;
    int index;
    int parentIdx;
    std::vector<int> childIndices;
    std::vector<int> childIdxsList;

    void setIndex(int index);
    void addChild(int childIndex);
    void optimize();
    void validate();
    std::string getIdString();
};
