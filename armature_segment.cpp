#include "armature_segment.h"

ArmatureSegment::ArmatureSegment(ShadowSkeleton* shadowSkel, BoneState* startingFrom, ArmatureSegment* parentSegment, bool isRootPinned) 
    : shadowSkel(shadowSkel),
      simTransforms(shadowSkel->simTransforms),
      forArmature(shadowSkel->forArmature),
      parentSegment(parentSegment),
      isRootPinned(isRootPinned),
      hasPinnedAncestor(parentSegment != nullptr && (parentSegment->isRootPinned || parentSegment->hasPinnedAncestor)),
      qcpConverger(MathUtils::DOUBLE_ROUNDING_ERROR, MathUtils::DOUBLE_ROUNDING_ERROR) {
    buildSegment(startingFrom);
    if (this->isRootPinned)
        this->wb_segmentRoot->setAsSegmentRoot();
    buildReverseTraversalArray();
    createHeadingArrays();
}

void ArmatureSegment::buildSegment(BoneState* startingFrom) {
    std::vector<WorkingBone*> segEffectors;
    std::vector<WorkingBone*> strandBones;
    std::vector<std::shared_ptr<ArmatureSegment>> subSgmts;
    std::vector<std::shared_ptr<ArmatureSegment>> childSgmts;
    BoneState* currentBS = startingFrom;
    bool finished = false;
    while (!finished) {
        std::shared_ptr<WorkingBone> currentWB = std::make_shared<WorkingBone>(currentBS, this);
        if (currentBS == startingFrom)
            this->wb_segmentRoot = currentWB;
        strandBones.push_back(currentWB);
        TargetState* target = currentBS->getTarget();
        if (target != nullptr || currentBS->getChildCount() > 1) {
            if (target != nullptr) {
                segEffectors.push_back(currentWB);
                if (target->getDepthFallOff() <= 0.0) {
                    this->wb_segmentTip = currentWB;
                    finished = true;
                }
            }
            if (finished) {
                for (int i = 0; i < currentBS->getChildCount(); i++)
                    childSgmts.push_back(std::make_shared<ArmatureSegment>(shadowSkel, currentBS->getChild(i), this, true));
            } else {
                for (int i = 0; i < currentBS->getChildCount(); i++) {
                    std::shared_ptr<ArmatureSegment> subseg = std::make_shared<ArmatureSegment>(shadowSkel, currentBS->getChild(i), this, false);
                    subSgmts.push_back(subseg);
                    subSgmts.insert(subSgmts.end(), subseg->subSegments.begin(), subseg->subSegments.end());
                    segEffectors.insert(segEffectors.end(), subseg->pinnedBones.begin(), subseg->pinnedBones.end());
                }
                finished = true;
                this->wb_segmentTip = currentWB;
            }
        } else if (currentBS->getChildCount() == 1) {
            currentBS = currentBS->getChild(0);
        } else {
            this->wb_segmentTip = currentWB;
        }
    }
    this->subSegments = subSgmts;
    this->pinnedBones.assign(segEffectors.begin(), segEffectors.end());
    this->childSegments = childSgmts;
    this->solvableStrandBones = strandBones;
}

void ArmatureSegment::buildReverseTraversalArray() {
    std::vector<WorkingBone*> reverseTraversalArray;
    for (WorkingBone* wb : solvableStrandBones) {
        if (wb->forBone->getStiffness() < 1.0) {
            reverseTraversalArray.push_back(wb);
        }
    }
    for (const auto& ss : subSegments) { // Use const auto& instead of auto&
        reverseTraversalArray.insert(reverseTraversalArray.end(), ss->reversedTraversalArray.begin(), ss->reversedTraversalArray.end());
    }
    this->reversedTraversalArray.assign(reverseTraversalArray.begin(), reverseTraversalArray.end());
}

void ArmatureSegment::createHeadingArrays() {
    std::vector<std::vector<double>> penaltyArray;
    std::vector<WorkingBone*> pinSequence; //TODO: remove after debugging
    recursivelyCreatePenaltyArray(penaltyArray, pinSequence, 1.0);
    int totalHeadings = 0;
    for (const auto& a : penaltyArray) {
        totalHeadings += a.size();
    }
    boneCenteredTargetHeadings.resize(totalHeadings);
    boneCenteredTipHeadings.resize(totalHeadings);
    uniform_boneCenteredTipHeadings.resize(totalHeadings);
    weights.resize(totalHeadings);
    int currentHeading = 0;
    for (const auto& a : penaltyArray) {
        for (double ad : a) {
            weights[currentHeading] = ad;
            boneCenteredTargetHeadings[currentHeading] = SGVec_3d();
            boneCenteredTipHeadings[currentHeading] = SGVec_3d();
            uniform_boneCenteredTipHeadings[currentHeading] = SGVec_3d();
            currentHeading++;
        }
    }
}

void ArmatureSegment::recursivelyCreatePenaltyArray(std::vector<std::vector<double>>& weightArray, std::vector<WorkingBone*>& pinSequence, double currentFalloff) {
    if (currentFalloff == 0) {
        return;
    } else {
        TargetState* target = this->wb_segmentTip->targetState;

        if (target != nullptr) {
            std::vector<double> innerWeightArray;
            weightArray.push_back(innerWeightArray);
            byte modeCode = target->getModeCode();
            innerWeightArray.push_back(target->getWeight() * currentFalloff);
            double maxPinWeight = target->getMaxPriority();

            if (maxPinWeight == 0)
                maxPinWeight = 1;

            if ((modeCode & AbstractIKPin::XDir) != 0) {
                double subTargetWeight = target->getWeight() * (target->getPriority(AbstractIKPin::XDir) / maxPinWeight) * currentFalloff;
                innerWeightArray.push_back(subTargetWeight);
                innerWeightArray.push_back(subTargetWeight);
            }
            if ((modeCode & AbstractIKPin::YDir) != 0) {
                double subTargetWeight = target->getWeight() * (target->getPriority(AbstractIKPin::YDir) / maxPinWeight) * currentFalloff;
                innerWeightArray.push_back(subTargetWeight);
                innerWeightArray.push_back(subTargetWeight);
            }
            if ((modeCode & AbstractIKPin::ZDir) != 0) {
                double subTargetWeight = target->getWeight() * (target->getPriority(AbstractIKPin::ZDir) / maxPinWeight) * currentFalloff;
                innerWeightArray.push_back(subTargetWeight);
                innerWeightArray.push_back(subTargetWeight);
            }
            pinSequence.push_back(wb_segmentTip);
        }
        
        double thisFalloff = target == nullptr ? 1 : target->getDepthFallOff();
        for (auto& s : subSegments) {
            s->recursivelyCreatePenaltyArray(weightArray, pinSequence, currentFalloff * thisFalloff);
        }
    }
}
