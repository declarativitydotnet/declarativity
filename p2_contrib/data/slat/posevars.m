function vars = posevars(s, cameras)
% function vars = posevars(model [, cameras])
% Returns the sets of pose variables for one or more cameras

global u; % the global universe
  
if ~exist('cameras','var')
  cameras = 1:length(get(model, 'calib'));
end

for i=1:length(cameras)
  cam = sprintf('%d',cameras(i));
  switch s.param
    case 'relative'
      vars(i) = domain([u.vector_timed_process(['xy' cam], 2).current,
                        u.vector_timed_process(['uv' cam], 2).current,
                        u.vector_timed_process(['pan' cam], 1).current]);
    case 'xy'
      vars(i) = domain([u.vector_timed_process(['ctr' cam], 2).current,
                        u.vector_timed_process(['pan' cam], 2).current]);
    otherwise
      error('Invalid parameterization');
  end
end
