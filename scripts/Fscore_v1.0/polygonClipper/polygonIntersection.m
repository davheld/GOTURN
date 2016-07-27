function P3 = polygonIntersection(P1, P2)
% P1 and P2 have the structure as follows:
% P1.x=[-1 1 1 -1]; P1.y=[-1 -1 1 1]; P1.hole=0;
% P2.x=[-2 0.8 0.4 -2]; P2.y=[-.5 -.2 0 .5]; P2.hole=0;
% 
P1.hole = 0;
P2.hole = 0;

type = 1; % A and B
P3 = PolygonClip(P1,P2,type);

%% visualization: uncomment the below lines if you want
% for i=1:3
%     eval(['p=P' num2str(i) ';'])
%     for np=1:length(p)
%         obj=patch(p(np).x,p(np).y,i);
%         if p(np).hole==1; set(obj,'facecolor','w'); end
%     end
% end


