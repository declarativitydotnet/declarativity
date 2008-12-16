% Next, test the pan/tilt approximation
load slat/problems
clear param2

param2.calib = prob3.calib(1);
param2.repr = 1
param2.cposi = [0 0 0];
param2.pani = 3;
param2.tilti = 4;
param2.trim = 1;

% Evaluate the projection at range of pans & tilts
[pans,tilts] = meshgrid(-pi:0.05:pi, -pi/2:0.05:pi/2);
x2 = [zeros(2, length(pans(:))); pans(:)'; tilts(:)'];
z = fnProjection(x2,param2);
z1 = pans;
z2 = pans;
z1(:) = z(1,:)';
z2(:) = z(2,:)';
figure(1);contour(pans,tilts,z1,'ShowText','on');
xlabel('pan');ylabel('tilt');
title('Variation of u with camera pan/tilt');
figure(2);contour(pans,tilts,z2,'ShowText','on');
xlabel('pan');ylabel('tilt');
title('Variation of v with camera pan/tilt');


muX2 = [0;0;0;-pi/4];
sigmaX2 = diag([1 1 pi/2 pi/8]);
%muX2 = [0;0;0;-0.4];
%sigmaX2 = diag([0.01 0.01 0.001 0.001]);
[mu2, S2, muY2, sigmaY2] = approximateJoint(muX2, sigmaX2, ...
    @fnProjection,  zeros(2), param2);
[u,v]=meshgrid(0:0.01:1, 0:0.01:2);

puv = u;
puv(:) = mvnpdf([u(:) v(:)], muY2', sigmaY2);

figure(3);contour(u,v,puv,'ShowText', 'on');
set(gca,'YDir', 'reverse')
title('Computed joint over the person observation');

[a2, A2, csigmaX2] = gJoint2Conditional([muX2; muY2], S2, 5:6)

% p(person|Y) where Y=[0.5; 0.5];
% p(parameters|y) 


