function vars = centervars(s, cameras)
% function vars = centervars(s, cameras)
% Returns the variables for the camera centers

global u; % The universe

if ~exist('cameras','var')
  cameras = 1:length(model.calib);
end

for i=1:numel(cameras)
  vars(i) = u.vector_timed_process(sprintf('c%d', cameras(i)), 2).current;
end
