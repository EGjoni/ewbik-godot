#include <vector>
#include <functional>
#include "skeleton_state.h"
#include "abstract_armature.h"
#include "abstract_axes.h"
#include "working_bone.h"
#include "armature_segment.h"

class ShadowSkeleton {
public:
    ShadowSkeleton(SkeletonState& skelState, AbstractArmature& forArmature);

    void solve(double dampening, int iterations, int stabilizationPasses, std::function<void(BoneState&)> notifier);

private:
    AbstractArmature* forArmature;
    std::vector<AbstractAxes*> simTransforms;
    AbstractAxes* shadowSpace;
    std::vector<WorkingBone*> traversalArray;
    ArmatureSegment* rootSegment;
    SkeletonState* skelState;

    void alignSimAxesToBoneStates();
    void alignBoneStatesToSimAxes();
    void alignBoneStatesToSimAxes(std::function<void(BoneState&)> notifier);
    void alignBone(WorkingBone& wb);
    void buildSimTransformsHierarchy();
    void buildArmatureSegmentHierarchy();
    void buildTraversalArray();
    void updateRates();
};
