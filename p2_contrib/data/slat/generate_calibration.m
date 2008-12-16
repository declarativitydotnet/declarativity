function calibration = generateCalibration(KK, param, imageSize)
% calibration = generateCalibration(KK, param, imageSize)
% Generates the calibration structure for a set of virtual cameras.
%
% KK:  internal calibration matrix (matrices)
% param: camera positions, pans, and tilts (5xn array)
% imageSize: size of the image (default: [640 480])

if ~exist('imageSize', 'var')
  imageSize = [640 480];
end

[stuff, n]=size(param);
%KK
%pos
%panTilt

for i=1:n,
  calib.pos  = param(1:3,i);
  calib.pan  = param(4,i);
  calib.tilt = param(5,i);
  calib.R  = pantilt2mat(calib.pan, calib.tilt);
  calib.T  = -calib.R*calib.pos;
  if iscell(KK)
    calib.KK= KK{i};
  else
    calib.KK = KK;
  end
  calib.imageSize = imageSize;
  calibration(i)=calib;
end
