function PolygonClip_example

    P1.x=[-1 1 1 -1]; P1.y=[-1 -1 1 1]; P1.hole=0;
    P1(2).x=[-1 1 1 -1]*.5; P1(2).y=[-1 -1 1 1]*.5; P1(2).hole=1;

    P2.x=[-2 0.8 0.4 -2]; P2.y=[-.5 -.2 0 .5]; P2.hole=0;
    P2(2).x=[2 0.8 0.6 1.5]; P2(2).y=[-1 0 0.3 1]; P2(2).hole=0;
%     P1.x=[-1 1 1 -1]; P1.y=[-1 -1 1 1]; P1.hole=0;
% 
%     P2.x=[-2 0.8 0.4 -2]; P2.y=[-.5 -.2 0 .5]; P2.hole=0;


    for type = 0:3
        subplot(2,2,type+1); box on
        switch type
            case 0; title('A-B')
            case 1; title('A.and.B (standard)')
            case 2; title('xor(A,B)')
            case 3; title('union(A,B)')
        end

        P3=PolygonClip(P1,P2,type);

        for i=1:3
            eval(['p=P' num2str(i) ';'])
            for np=1:length(p)
                obj=patch(p(np).x,p(np).y,i);
                if p(np).hole==1; set(obj,'facecolor','w'); end
            end
        end

    end
