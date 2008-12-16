function problem=generateSlatNSH(KK,varargin)
%problem=generateSlatOverhead(KK,m,n,height,varargin)
%the optional parameters fps, mps are passed to generateSlatProblem

%[x,y]=meshgrid(xs,ys);
%n=length(x(:));

ceiling=2.7;

% The lower edge
pos1 = [25:-2:5;
        ones(1,11)*4.5;
        ones(1,11)*2;
        ones(1,11)*(-pi/2);
        ones(1,11)*(-0.3)];
pos2 = [25 15; 3.65 3.65; ceiling ceiling; 0 0; -0.2 -0.2];
[x,y]=meshgrid(1:3, 3:13);
n=length(x(:));
pos3 = [x(:)';y(:)';ones(1,n)*ceiling;rand(1,n)*pi/2;ones(1,n)*(-pi/2)];
pos4 = [2 12; 13 9; ceiling ceiling; pi pi; -0.2 -0.2];
pos5 = [5:2:25;
        ones(1,11)*14.8-5*[0 0 0 0 1 1 1 1 1 1 1];
        ones(1,11)*2;
        ones(1,11)*(-pi/2);
        ones(1,11)*(-0.3)];
[x,y]=meshgrid(27:29, 3:13);
n=length(x(:));
pos6 = [x(:)';y(:)';ones(1,n)*ceiling;rand(1,n)*pi/2;ones(1,n)*(-pi/2)];

pos=[pos1 pos2 pos3 pos4 pos5 pos6];

%A grid of cameras at x,y,height, oriented in random directions, facing
%down
calib=generateCalibration(KK, pos);
range=[-2 33 -2 17];
kdoorl=150+300+300;
kdoorr=kdoorl+48
kitchenr=kdoorr+250;
width=kdoorr+250+60+160;  % 60?
h=220+160+70+135;
turnl=150+300+72;
deskt=100;
loungel=turnl+300+90+42+80;
geometry = ...
  {[kdoorl 100; 0 100; 0 h; turnl h; turnl h-190; loungel h-190; ...
    loungel h; width h; width h-135]*0.025, ...
   [width 220; width 220+160]*0.025,...
   [150 deskt+80; kitchenr deskt+80; kitchenr h-260; 450 h-260; ...
    450 h-78; 150 h-78; 150 180]*0.025, ...
   [kdoorr 100; width 100]*0.025};
problem = generateSlatProblem(calib, 'range', range, ...
  'geometry', geometry, varargin{:});
dx=x(:)*ones(1,n)-ones(n,1)*(x(:)');
dy=y(:)*ones(1,n)-ones(n,1)*(y(:)');
problem.adjacency=sqrt(dx.^2+dy.^2)<=3;

