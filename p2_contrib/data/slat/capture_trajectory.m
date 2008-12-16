function prob = capture_trajectory(calib, varargin)
% prob = capture_trajectory(calib, range, ...)
% Generates the problem using the calibration parameters and
% queries the user for a trajectory

%if exist('description', 'var')
%  prob.description = description;
%end

[range, ...
 geometry, ...
 fps, ...
 mps, ...
 height] = process_options(varargin, 'range', [0 1 0 1], ...
                                     'geometry', {}, ...
                                     'fps', 10, ...
                                     'mps', 0.5, ...
                                     'height', 1.8);

prob.ncams = length(calib);
prob.calib = calib;
prob.range = range;
prob.geometry = geometry;
prob.height = height;

plotcams(prob,'visibility',1);
hold on
button = 1;
n=0;
while button == 1
    [xi,yi,button] = ginput(1);
    plot(xi,yi,'ro')
    n = n+1;
    xy(:,n) = [xi;yi];
end
hold off;

% Compute the prob parameters
spf = 1/fps;

xys = spline(1:n,xy,1:0.1:n);
len = sum(sqrt(sum((xys(:,2:end)-xys(:,1:end-1)).^2,1)));
npm = n/len;
nps = npm * mps;
npf = nps * spf;
nsteps = length(1:npf:n);

prob.dt      = spf;
prob.nsteps  = nsteps;
prob.time    = spf*(1:nsteps);
prob.pos     = spline(1:n, xy, 1:npf:n);
prob.pos     = [prob.pos; repmat(height, 1, nsteps)];
% prob.obs_var = 9;

%[x, y, button] = ginput(1);
%n=0;
%tic
%while button==1
%    plot(x,y,'ro')
%    n = n+1;
%    prob.pos(:,n) = [x;y;1.8]; % a person is 1.8 m meteres tall
%    prob.time(n) = toc;
%    [x, y, button] = ginput(1);
%end
%prob.nSteps = n;

prob = generate_observations(prob, 9);

% Plot the prob - now with the path & the observations
plotdata(prob);
