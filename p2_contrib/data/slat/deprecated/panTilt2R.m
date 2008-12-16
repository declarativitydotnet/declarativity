function R=panTilt2R(pan, tilt)
% R=panTilt2R(pan, tilt)
% Computes the rotation matrix for a camera with given pan and tilt
% values (relative to the default orientation, facing center along
% the X axis horizontally).

R=[0 1 0; 0 0 -1; -1 0 0];
R=getCameraRotationMatrix(pan, tilt)*R;
