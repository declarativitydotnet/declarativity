function info = gMutualInformation(cov, varsx, varsy)
varsxy=[varsy varsx];
info = gEntropy(cov(varsx,varsx),[]) - ...
       gEntropy(cov(varsxy,varsxy), 1:length(varsy));