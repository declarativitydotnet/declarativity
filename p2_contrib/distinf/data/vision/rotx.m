function R = rotx(theta)
% function R=rotx(theta)
% Returns the matrix that represents rotation around the x axis by theta

R = eye(3);
R(2:3,2:3) = [cos(theta) -sin(theta); sin(theta) cos(theta)];
