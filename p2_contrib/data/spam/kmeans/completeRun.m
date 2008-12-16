function [c_avg] = completeRun(smat, len, dom, k, split)
    i = 1;
    cTotal= [];
    csize=size(cTotal,1);
    c_avg = zeros(k,dom);
    count = 1
    while 1
        if((i+split-csize-1) <= len)
            count
            i
            i+split-csize-1
            if csize~=0
                M = smat(i:i+(split-csize-1),:);
                ls = size(M,1)+1
                le = ls+csize-1
                M(ls:le,:) = csize;
            else
                M=smat(i:i+split-1,:);
            end
            i = i+split-csize;
            [c,c_avg,cTotal,g,itr]=kMeans(M,k,1,0);
            csize=size(cTotal,1)    
            count = count+1
        else
            break;
        end
    end
