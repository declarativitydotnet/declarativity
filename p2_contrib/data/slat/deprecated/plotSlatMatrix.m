function plotSlatMatrix(matrix, options)
% plotSlatMatrix(matrix, options)
% Plots a matrix of solutions, with parametrization on one axis and
% linearization on the other. 
%
% Options can include any options, accepted  by plotSlatProblem, and
%   's' - swaps axes
% Default: 'c'
%

if ~exist('options', 'var')
  options='c';
end

m=size(matrix.soln,1); % parametrizations
n=size(matrix.soln,2); % linearizations

% if swap_axes,
%   subplot(m,n,1);
% else
%   subplot(n,m,1);
% end

for i=1:m, 
  for j=1:n,
    if ~any(options=='s')
      subplot(m, n, (i-1)*n+j);
    else
      subplot(n, m, (j-1)*m+i);
    end

    soln=matrix.soln{i,j};
    if length(soln)>0
      % Determine the # of valid steps
      nSteps=matrix.prob.nSteps;
      for t=1:size(soln.xt,2)
        if length(soln.Pt{t})==0
          nSteps=t-1;
          break;
        end
      end
      plotSlatSolution(matrix.prob, matrix.model{i}, soln, nSteps,...
        [options 'f']);
    end
    
    % Plot the labels
    if ~any(options=='s')
      if j==1,
        ylabel(matrix.param{i});
      end
      if i==m,
        xlabel(matrix.linzn{j});
      end
    else
      if i==1,
        ylabel(matrix.param{j});
      end
      if j==n,
        xlabel(matrix.linzn{i});
      end
    end
  end
end

