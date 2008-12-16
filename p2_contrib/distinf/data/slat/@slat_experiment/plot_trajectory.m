function plot_trajectory(object, varargin)

soln = latest_solution(object);
for i=1:numel(soln)
  disp(['Plotting the solution from ' datestr(soln(i).created)]);
end

plot_trajectory(soln, varargin{:});
