function areas = polygonIntersectionArea(P1, P2)
%   areas = polygonIntersectionArea(P1, P2)
% Compute the areas of the intersection between two polygons.
% P1 and P2 are structures containing information about the two polygons
% Structure: P.x and P.y
%   x: [x1 x2 ... xn]
%   y: [y1 y2 ... yn]
%       where (xi, yi) is the coordinate of the i vertice in the polygon
% Output: areas of the intersection of the two polygon. be aware that there
% may be more than 1 intersections
% 

% P1.x=[-1 1 1 -1]; P1.y=[-1 -1 1 1]; 
% P1.hole = 0;

% P2.x=[-2 0.8 0.4 -2]; P2.y=[-.5 -.2 0 .5]; 
% P2.hole = 0;

type = 1; % A and B
P3 = PolygonClip(P1,P2,type);

if(isempty(P3) )
    areas = 0;
end

for np=1:length(P3)
    p = P3(np);
    pArea = polyarea(p.x, p.y);
%     fprintf('Intersection %i: %f', np, pArea);
    areas(np) = pArea;
end



