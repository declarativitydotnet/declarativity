function R = roty(theta)
% function R=roty(theta)
% Returns the matrix that represents rotation around the y axis by theta

R = eye(3);
R([3 1],[3 1]) = [cos(theta) -sin(theta); sin(theta) cos(theta)];
