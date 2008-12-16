function kl=gaussianKL(meanp, covp, meanq, covq)
% kl=gKL(mean1, cov1, mean2, cov2)

if size(meanp)~=1 | size(meanq)~=1
  error('meanp and meanq must be column vectors');
end

if any([size(covp) size(covq) length(meanp)]~=length(meanq))
  error('invalid matrix dimensions');
end

kl =  0.5*(log(det(covq))-log(det(covp))) ...
    + 0.5*trace(inv(covq)*covp) ...
    + 0.5*(meanp-meanq)'*inv(covq)*(meanp-meanq) ...
    - 0.5*length(meanp);
