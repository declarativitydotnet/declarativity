function [vel,acc]=computeMotionStatistics(problem, dt)
if isstruct(problem)
  pos = problem.pos;
  time= problem.time;
else
  pos = problem;
  time = (1:size(pos,2))*dt;
end
vel=(pos(1:2,2:end)-pos(1:2,1:end-1))./ ...
    (ones(2,1)*(time(2:end)-time(1:end-1)));
acc=(vel(:,2:end)-vel(:,1:end-1))./ ...
    (ones(2,1)*(time(3:end)-time(1:end-2))/2);
  
disp('Standard deviation of acceleration: ')
std(acc')
disp('Standard deviation of acceleration in each time step: ')
std(acc')*(time(2)-time(1))
plot(acc(1,:)*(time(2)-time(1)),acc(2,:)*(time(2)-time(1)),'.');
