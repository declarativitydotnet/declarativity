function prob=generateSlatIntel(simulation, height, extrinsic) %, ids)
%function prob=generateSlatIntel(simulation, height, extrinsic) %, ids)
% prob=generateSlatIntel(simulation)
%   or
% prob=generateSlatIntel(simulation, ids)
% A simulated experiment with data from cameras at IRP

if ~exist('extrinsic','var')
  extrinsic=0;
end

[camids,locids]=readCameraTable;

% Extrinsic parameters: x,y,z (inches), pan, tilt (radians)
ext = [ ...
0 0 0 pi/2 -pi/2 ; ...                1
25.5 228.75 116.5 pi/2 -pi/2 ; ...    2
74 226.5 116.5 -pi -pi/2 ; ...        3
119.75	226.25	116.5 0 -pi/2 ; ...   4
214.75	240	117	 pi/2 -pi/4 ; ...     5
150 	214	116.5 -pi/2 -pi/2 ; ...     6
44.75	186	133 pi/2 -pi/2 ; ...        7
80.75	177	133 -pi/2 -pi/2 ; ...       8
120	186	133 -pi -pi/2 ; ...           9
156	184	133 pi/2 -pi/2 ; ...          10
198.5	184	133 pi/2 -pi/2 ; ...        11
237.75	184.5	133 0 -pi/2 ; ...       12
184.75	240	117	 pi/2 -pi/4 ; ...     13
24	68.75	133 pi/2 -pi/2 ; ...        14
80.5	137	133 pi/2 -pi/2 ; ...        15
105.75	29	133 -pi/2 -pi/2 ; ...     16
64.5	15.5	133 pi/2 -pi/2 ; ...      17
29	29	133 -pi -pi/2 ; ...           18
192	59.5	133 -pi -pi/2 ; ...         19
150	67.5	133 -pi/2 -pi/2 ; ...       20  
109.5	71	133 pi/2 -pi/2 ; ...        21
64.25	73	133 -pi/2 -pi/2 ; ...       22
132.5	126.75	133 -pi/2 -pi/2 ; ...   23
170.25	137	133 -pi/2 -pi/2 ; ...     24
201.75	125	133 pi/4 -pi/2 ; ...      25
151	258.5	116.5 -pi/2 -pi/2 ; ...     26
21.25	264	133 0 -pi/2 ; ...           27
69.25	257.25	133 pi/2 -pi/2 ; ...    28
115.75	257.5	133 pi/2 -pi/2 ; ...    29
24	325.25	133 pi/2 -pi/2 ; ...      30
74.25	322.5	133 pi/2 -pi/2 ; ...      31
232.5	325	133 -pi/2 -pi/2 ; ...       32
165	290.75 116.5 0 -pi/2 ; ...        33
102.25	283.25	133 -pi/2 -pi/2 ; ... 34
41.75	361	133	 -pi/2 -pi/2 ; ...      35
127.5	360	133 -pi/2 -pi/2 ; ...       36
97	359	133	-pi -pi/2 ; ...           37
278.25	326	133  0 -pi/2 ; ...        38
115	327.5	133 -pi/4 -pi/2 ; ...       39
146.75	334.5	116.5 pi/2 -pi/2 ; ...  40
195.25	324	133 pi/2 -pi/2]';% ; ...  41
%36.5	166.5	96.5  pi/2 ? ; ...    42
%8.5	209.5	96.5 -pi ? ; ...      43
%6 86	  107 -pi ? ; ...           44
%9.5 34	107 -pi ?];               45


ext(1:3,:)=ext(1:3,:)*0.0254;

ext = ext(:,locids);

% Intrinsic parameters
%KK = ...
% [461.4538         0  329;...
%         0  461.0651  240;...
%         0         0    1.0000];

%for i=1:length(KK), 
%  KK{i}(1:2,1:3)=KK{i}(1:2,1:3)/2; % To account for the fact that images are 320 x 240
%end
    
ids = camids;

if extrinsic
  
  for i=1:length(ids),
    a = load(sprintf('~/repo/testbeds/cameras/calib/cam%02d/Calib_Results', ids(i)));
    KK{i} = a.KK;
    a = load(sprintf('~/repo/testbeds/cameras/calib/cam%02d/extrinsic', ids(i)));
    ptr=R2ptr(a.Rc_ext);
    disp(sprintf('cam%02d: ptr-calibrate_ptr (in degrees) = ', ids(i)));
    R{i}=a.Rc_ext;
    (ext(4:5,i)-ptr(:))'/pi*180
    ext(4,i)=ptr(1);
  end

else  
  
  for i=1:length(ids),
    a = load(sprintf('~/repo/testbeds/cameras/calib/cam%02d/Calib_Results', ids(i)));
    KK{i} = a.KK;
  end

end

calib=generate_calibration(KK, ext, [640 480]);
range=[-1 6 1  11];

%if ~exist('ids', 'var')
%  ids=1:length(calib);
%  calib=calib(ids);
%else
%  if all(ids == 0),
%    ids = setdiff(2:41, [36 37 42 4 11 5 8 9 19 20 16 32 39]);  
%  end
%  ids = setdiff();
%end

if simulation
  prob=capture_trajectory(calib, 'range', range, 'height', height);
else
  prob.nCams=length(calib);
  prob.calib=calib;
  %prob.R=R;
  prob.range=range;
  prob.sigma_u=3;
  prob.sigma_v=3;
  prob.height=height;
end
prob.cameraid = ids;
%   prob.geometry = ...
%         {[0 0; -12 0; -12 -9; 0 -9; pi/2 -pi/2]};
%   prob.adjacency=[1 1 0 1 0 1; 1 1 1 1 1 1; 0 1 1 1 1 0; 1 1 1 1 1 1; 0 1 1 1 1 1; 1 1 0 1 1 1];


function [camid,locid]=readCameraTable
f = fopen('~/repo/testbeds/cameras/intel50/slat-machine-camera-table');
str = fgetl(f); % skip the first line
str = fgetl(f);
i=1;
while ischar(str),
  [start,stop]=regexp(str,'\S+');
  
  if length(start)>0
    camid(i) = str2num(str(start(3)+3:stop(3)));
    locid(i) = str2num(str(start(4):stop(4)));
    i=i+1;
  end
  str = fgetl(f);
end
