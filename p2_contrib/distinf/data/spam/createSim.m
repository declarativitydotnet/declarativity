function [sim,pref] = createSim(mat, n, dom)
    if(size(mat,2)==3)
        smat = sparse(mat(:,1), mat(:,2), mat(:,3), n, dom);
        sim = normprod(smat,smat');    
        pref = median(sim(:,:),2);
    else
        smat = sparse(mat);  
        sim = normprod(smat,smat');    
        pref = median(sim(:,:),2);
    end
end