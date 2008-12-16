function [node_format, offset] = target_info(target)
% This function is not used anymore
  
switch target
 case 'localhost'
  node_format = 'localhost:%5d';
  offset = 10000;
 case 'emulab'
  node_format = 'node-%d.DOMAIN';
  offset = 0;
 otherwise
  assert(false);
end
