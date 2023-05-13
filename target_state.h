#pragma once
#include <string>
#include <vector>

class SkeletonState;

class TargetState {
public:
    TargetState(const std::string &id, const std::string &transform_id, const std::string &forBone_id, std::vector<double> priorities, double depthFalloff, double weight, SkeletonState &skeletonState);

    void setIndex(int index);
    void optimize();
    TransformState* getTransform();
    void validate();
    std::string getIdString();
    uint8_t getModeCode();
    double getDepthFalloff();
    double getWeight();
    int getIndex();
    double getPriority(int basisDirection);
    double getMaxPriority();

private:
    std::string id;
    std::string transform_id;
    std::string forBone_id;
    uint8_t modeCode;
    std::vector<double> priorities;
    double depthFalloff;
    double weight;
    int index;
    int transformIdx;
    int forBoneIdx;
    SkeletonState &skeletonState;

    void init(const std::string &id, const std::string &transform_id, const std::string &forBone_id, const std::vector<double> &priorities, double depthFalloff, double weight);
};
