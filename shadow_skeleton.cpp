#include "shadow_skeleton.h"

ShadowSkeleton::ShadowSkeleton(SkeletonState& skelState, real_t dampening) {
    this->base_dampening = dampening; 
    buildSimTransformsHierarchy();
    buildArmatureSegmentHierarchy();
    buildTraversalArray();
}

void ShadowSkeleton::solve(double dampening, int iterations, int stabilizationPasses, std::function<void(BoneState&)> notifier) {
    int endOnIndex = traversalArray.size() - 1;
    int tipIndex = 0;
    ArmatureSegment* forSegment = rootSegment;

    // These might need adjustments based on your actual implementation.
    iterations = iterations == -1 ? forArmature->getDefaultIterations() : iterations;
    stabilizationPasses = stabilizationPasses == -1 ? forArmature->getDefaultStabilizingPassCount() : stabilizationPasses;

    alignSimAxesToBoneStates();

    for (int i = 0; i < iterations; i++) {
        for (int j = 0; j <= endOnIndex; j++) {
            traversalArray[j]->pullBackTowardAllowableRegion(i, iterations);
        }
        for (int j = 0; j <= endOnIndex; j++) {
            traversalArray[j]->fastUpdateOptimalRotationToPinnedDescendants(stabilizationPasses,
                j == endOnIndex && endOnIndex == traversalArray.size() - 1);
        }
    }
    if (notifier == nullptr) {
        alignBoneStatesToSimAxes();
    } else {
        alignBoneStatesToSimAxes(notifier);
    }
}

void ShadowSkeleton::alignSimAxesToBoneStates() {
    std::vector<TransformState*> transforms = skelState->getTransformsArray();
    for (int i = 0; i < transforms.size(); i++) {
        this->simTransforms[i]->getLocalMBasis().set(transforms[i]->translation, transforms[i]->rotation, transforms[i]->scale);
        this->simTransforms[i]->_exclusiveMarkDirty(); // Assuming _exclusiveMarkDirty() exists in IKNode3D
    }
}

void ShadowSkeleton::alignBoneStatesToSimAxes() {
    for (int i = 0; i < traversalArray.size(); i++) {
        WorkingBone* wb = traversalArray[i];
        alignBone(*wb);
    }
}

void ShadowSkeleton::alignBoneStatesToSimAxes(std::function<void(BoneState&)> notifier) {
    for (int i = 0; i < traversalArray.size(); i++) {
        WorkingBone* wb = traversalArray[i];
        alignBone(*wb);
        notifier(*(wb->forBone));
    }
}

void ShadowSkeleton::alignBone(WorkingBone& wb) {
    BoneState* bs = wb.forBone;
    TransformState* ts = bs->getTransform();
    // Assuming the translation, rotation, and scale are accessible as properties or methods.
    ts->translation = wb.simLocalAxes.localMBasis.translate.get();
    ts->rotation = wb.simLocalAxes.localMBasis.rotation.toArray();
}

void ShadowSkeleton::buildSimTransformsHierarchy() {
    int transformCount = skelState->getTransformCount();
    if (transformCount == 0) return;

    this->simTransforms.resize(transformCount);
    this->shadowSpace = //IFire, make an Identity IKNode3d here. 

    for (int i = 0; i < transformCount; i++) {
        TransformState* ts = skelState->getTransformState(i);
        IKNode3D* newTransform = shadowSpace->clone();
        // Detach from parent and set to identity
        newTransform->set_parent(Ref<IKNode3D>());
        newTransform->set_transform(Transform3D(ts->translation, ts->rotation, ts->scale));

        this->simTransforms[i] = newTransform;
    }
    for (int i = 0; i < transformCount; i++) {
        TransformState* ts = skelState->getTransformState(i);
        int parTSidx = ts->getParentIndex();
        IKNode3D* simT = this->simTransforms[i];
        if (parTSidx == -1) {
            simT->set_parent(Ref<IKNode3D>());
        } else {
            simT->set_parent(this->simTransforms[parTSidx]);
        }
    }
}

void ShadowSkeleton::buildArmatureSegmentHierarchy() {
    BoneState* rootBone = skelState->getRootBonestate();
    if (rootBone == nullptr) return;
    rootSegment = new ArmatureSegment(this, rootBone, nullptr, false);
}

void ShadowSkeleton::buildTraversalArray() {
    if (rootSegment == nullptr) return;
    std::vector<ArmatureSegment*> segmentTraversalArray = rootSegment->getDescendantSegments();
    std::vector<WorkingBone*> reversedTraversalArray;
    for (ArmatureSegment* segment : segmentTraversalArray) {
        // Assuming reversedTraversalArray is accessible as a property or method in ArmatureSegment
        reversedTraversalArray.insert(reversedTraversalArray.end(), segment->reversedTraversalArray.begin(), segment->reversedTraversalArray.end());
    }
    traversalArray.resize(reversedTraversalArray.size());
    int j = 0;
    for (int i = reversedTraversalArray.size() - 1; i >= 0; i--) {
        traversalArray[j] = reversedTraversalArray[i];
        j++;
    }
}

void setDampening(real_t dampening) {
    this->base_dampening = dampening; 
    this->updateRates();
}

void ShadowSkeleton::updateRates() {
    for (int j = 0; j < traversalArray.size(); j++) {
        traversalArray[j]->updateCosDampening(); // Assuming this method exists in WorkingBone
    }
}