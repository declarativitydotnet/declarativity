function a = genmog(n, p, mu, sigma)

c = mnrnd(n, p);

a = [];
for i=1:length(c)
  r = mvnrnd(mu{i}, sigma{i}, c(i));
  a = [a; r];
end
