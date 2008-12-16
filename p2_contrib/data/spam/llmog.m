function ll=llmog(x, s)
% function X=llmog(x, s)
% Evaluates the log-likelihood of a dataset for a mixture of Gaussians

l = pmog(x, s);
ll = log(sum(p,2));

