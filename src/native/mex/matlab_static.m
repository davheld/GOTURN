function [] = matlab_static()

cleanup = onCleanup(@() exit() );

traxserver('setup', 'polygon', 'path');

memory = [0 0 0 0];

while 1

    [image, region] = traxserver('wait');

    if isempty(image)
		break;
	end;

	if ~isempty(region)
        memory = region;
    end

	traxserver('status', memory);
end

