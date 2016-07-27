function evaluate_all(annotations_folder, output_folder)

fprintf('%s\n', output_folder);
output_files = dir(output_folder);

Fscores = [];
categories = [];

threshes = [0.5 0.7 0.9];
scores = zeros(length(threshes),1);

for j = 1:length(threshes)
    thresh = threshes(j);
    for i = 1:length(output_files)
        file = output_files(i);
        if (file.isdir)
            continue;
        end
        
        if (file.name(1) == '.')
            continue
        end

        category = strtok(file.name, '_');
        categories = [categories category];

        output_file = [output_folder '/' file.name];
        annotation_file = [annotations_folder '/' category '/' file.name '.ann'];     
        Fscore = quantitativeEvaluationFScore_poly(output_file, annotation_file, thresh);
        if (isnan(Fscore))
            Fscore = 0;
        end      
        Fscores = [Fscores; Fscore];
    end
    scores(j) = mean(Fscores);
end

for i = 1:length(threshes)
   fprintf('Thresh: %f, Mean: %f\n', threshes(i), scores(i)); 
end

end

