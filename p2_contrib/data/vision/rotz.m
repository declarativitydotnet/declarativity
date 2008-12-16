function R = rotz(theta)
% function R=rotz(theta)
% Returns the matrix that represents rotation around the z axis by theta

R = eye(3);
R(1:2,1:2) = [cos(theta) -sin(theta); sin(theta) cos(theta)];
