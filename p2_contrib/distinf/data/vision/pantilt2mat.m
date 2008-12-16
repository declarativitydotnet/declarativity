function R=pantilt2mat(pan, tilt)
% function R=pantilt2mat(pan, tilt)
R = (rotz(pan)*rotx(-tilt))';
