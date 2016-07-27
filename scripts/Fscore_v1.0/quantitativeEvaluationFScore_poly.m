%% The function takes three inputs. 
%% trk_output_File is the path to the tracking output file in format of [frameNumber X1(topLeft) Y1(topLeft) Width Height]
%% ann_File in the format of ALOV++ released annotations 

function fs = quantitativeEvaluationFScore_poly(trk_output_File, ann_File, thetas)

%% Read OutPut
%fprintf('Reading output from %s\n', trk_output_File);
% [outputFile] = textread(trk_output_File); % [frameId c1 r1 c2 r2]
fid = fopen(trk_output_File);
[outputFile1, num_chars] = textscan(fid, '%d %f %f %f %f'); % [frameId c1 r1 c2 r2]

if (num_chars == 0)
    fprintf('Error - cannot read file: %s\n', trk_output_File);
    fs = 1;
    return;
end

for i = 1:length(outputFile1)
    outputFile(:,i) = double(cell2mat(outputFile1(i)));
end

outputFile(:,4)=outputFile(:,2)+outputFile(:,4);
outputFile(:,5)=outputFile(:,3)+outputFile(:,5);

%% Read Annotation
[ann_File] = textread(ann_File); % varying [frameId c1 r1 c2 r2 ... cn rn]
addpath(genpath('.\polygonClipper\'));

%% scan through all the frames having annotation in annFile
NoAnnFrames = size(ann_File, 1);
frameIndex = outputFile(:,1);
errors = [];

for i=1:NoAnnFrames 
    ann = ann_File(i, :);
    ann = trimDoubleZeros(ann);
    frameId = ann(1);
    annPoly.x = ann(2:2:end);
    annPoly.y = ann(3:2:end);
    
    % find the corresponding frame in the trackingResultFile
    corrFrameId = find(frameIndex==frameId);
    if(size(corrFrameId,1)==0)%not find any corresponding index
        % do somthing here????
        errors(i,1) = frameId;
        errors(i,2) = NaN;
        continue;
    end
    
    trackedPos = outputFile(corrFrameId, 2:5); % [c1 r1 c2 r2]
    trackedPoly.x = [trackedPos(1) trackedPos(3) trackedPos(3) trackedPos(1)];
    trackedPoly.y = [trackedPos(2) trackedPos(2) trackedPos(4) trackedPos(4)];
    % compute the overlapping area
    annArea = polyarea(annPoly.x, annPoly.y);
    trackedArea = polyarea(trackedPoly.x, trackedPoly.y);
    sharedArea = polygonIntersectionArea(annPoly, trackedPoly);
    sharedArea = sum(sharedArea);
    
    errors(i,1) = frameId;
    errors(i,2) = sharedArea / (annArea + trackedArea - sharedArea); % pascal overlapping
end

if(size(errors,1)==0)
    fprintf('\n WARNING: there is NO performance evaluation for this video!!!\n');
    fs = [];
else
    overlaps = errors(:, 2);
    fs = [];
    for i=1:numel(thetas)
        theta = thetas(i);
        FN = nnz(isnan(overlaps)); % no track box is ass. with a gt
        TP = nnz(overlaps >= theta);
        FP = nnz(overlaps < theta); % a track box is not ass. with a gt
        FN = FN + FP; % cvpr12 formula [1]
        P = TP / (TP + FP);
        R = TP / (TP + FN);
        fs_ = fscoreCompute(P, R);
        fs = [fs fs_];
    end
end
end % of the main function 

%% -------------------------------------------------
function ann = trimDoubleZeros(ann)
i = numel(ann);
while ann(i) == 0 && i > 0
    if abs(ann(i) + ann(i-1)) == 0
       ann([i (i-1)]) = []; 
       i = i - 1;
    end
    
    i = i - 1;
end

end
