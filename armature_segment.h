#include "shadow_skeleton.h"
#include "bone_state.h"
#include "target_state.h"
#include "working_bone.h"
#include "qcp.h"
#include <vector>
#include <memory>

class ArmatureSegment {
private:
    ShadowSkeleton* shadowSkel;
    AbstractAxes** simTransforms;
    AbstractArmature* forArmature;
    std::vector<std::shared_ptr<ArmatureSegment>> subSegments;
    std::vector<std::shared_ptr<ArmatureSegment>> childSegments;
    std::vector<WorkingBone*> solvableStrandBones;
    std::vector<WorkingBone*> allStrandBones;
    std::vector<WorkingBone*> solvableSegmentBones;
    std::vector<WorkingBone*> allSegmentBones;
    std::vector<WorkingBone*> reversedTraversalArray;

    WorkingBone* wb_segmentRoot;
    std::vector<SGVec_3d> boneCenteredTargetHeadings;
    std::vector<SGVec_3d> boneCenteredTipHeadings;
    std::vector<SGVec_3d> uniform_boneCenteredTipHeadings;
    std::vector<double> weights;
    std::vector<WorkingBone*> pinnedBones;
    bool isRootPinned = false;
    bool hasPinnedAncestor = false;
    double previousDeviation = std::numeric_limits<double>::infinity();
    QCP qcpConverger;

    WorkingBone* wb_segmentTip;
    ArmatureSegment* parentSegment;

public:
    ArmatureSegment(ShadowSkeleton* shadowSkel, BoneState* startingFrom, ArmatureSegment* parentSegment, bool isRootPinned);
    void buildSegment(BoneState* startingFrom);
    void buildReverseTraversalArray();
    void createHeadingArrays();
    void recursivelyCreatePenaltyArray(std::vector<std::vector<double>>& weightArray, std::vector<WorkingBone*>& pinSequence, double currentFalloff);
    std::vector<std::shared_ptr<ArmatureSegment>> getDescendantSegments();
    double getManualMSD(std::vector<SGVec_3d>& locTips, std::vector<SGVec_3d>& locTargets, std::vector<double>& weights);
};
