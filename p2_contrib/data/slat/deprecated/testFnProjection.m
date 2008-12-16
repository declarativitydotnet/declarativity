load slat/problems
clear param
clear param2

% First, test the position approximation
param.calib = prob3.calib(1);
param.repr = 1
param.cposi = [3 4 0];
param.pani = 0;
param.tilti = 0;
param.trim = 1;  
 % Specifies whether or not to bound the results at 0.5 units
 % from the boundary

[xx,yy] = meshgrid(-3:0.1:6,-3:0.1:3);
x = [zeros(2, length(xx(:))); xx(:)'; yy(:)'];
z = fnProjection(x,param);
z1 = yy;
z2 = yy;
z1(:) = z(1,:)';
z2(:) = z(2,:)';

figure(1);contour(xx,yy,z1,'ShowText','on');
xlabel('x');ylabel('y');
title('Variation of u with camera position');
figure(2);contour(xx,yy,z2,'ShowText','on');
xlabel('x');ylabel('y');
title('Variation of v with camera position');

% The joint over the prior camera and person positions
muX = [0;0;0;0];
sigmaX = diag([1e-2 1e-2 2 2]);
%sigmaX = diag([1e-3 1e-3 100 100]);

%muX = [0;0;-2;0];
%sigmaX = diag([1e-6 1e-6 100 100]);

%muX = [0;0;2;0.5];
%sigmaX = diag([1e-4 1e-4 1 1]);


%[muX, sigmaX] = conditionOnDiscreteSampling(muX, sigmaX, @fnVisible, 1, param)
[muX2, sigmaX2,pts,wts] = visibilitySampling(muX, sigmaX, param, [0.1818;0.3289], ...
    diag([3/640 3/480]).^2);
    %    diag([0.1818 0.3289].^2));

[u,v]=meshgrid(-5:0.05:5, -5:0.05:5);
puv = u;
puv(:) = mvnpdf([u(:) v(:)], muX2(3:4)', sigmaX2(3:4,3:4));

figure(3);contour(u,v,puv,'ShowText', 'on');
%set(gca,'YDir', 'reverse')
title('Adjusted joint over the prior camera position.');


[mu, S, muY, sigmaY] = approximateJoint(muX, sigmaX, @fnProjection, ...
    zeros(2), param);

%[u,v]=meshgrid(0:10:param.calib.imageSize(1), ...
%    0:10:param.calib.imageSize(2));
[u,v]=meshgrid(0:0.01:1, 0:0.01:2);

puv = u;
puv(:) = mvnpdf([u(:) v(:)], muY', sigmaY);

figure(4);contour(u,v,puv,'ShowText', 'on');
set(gca,'YDir', 'reverse')
title('Computed joint over the person observation');
[a, A] = gJoint2Conditional(mu, S, 1:4)
[b, B, csigmaX] = gJoint2Conditional([muX; muY], S, 5:6)


