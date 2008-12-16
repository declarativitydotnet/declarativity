function lik=lmog(x,s)
k=length(s.mu);
for i=1:k;
  lik(:,i) = mvnpdf(x,s.mu{i},s.sigma{i});
end

