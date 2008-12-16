function p=pmog(x,s)
k=length(s.mu);
for i=1:k;
  p(:,i) = mvnpdf(x,s.mu{i},s.sigma{i})*s.alpha(i);
end

