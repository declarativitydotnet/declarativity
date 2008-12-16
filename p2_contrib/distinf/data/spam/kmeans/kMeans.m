function [c,c_avg,c_total,g,itr] = kmeans(m,k,isRand, isSimilarity)
if nargin<3,        isRand=0;   isSimilarity=0; end
if nargin<2,        k=1;        end
itr=0;   
[maxRow, maxCol]=size(m);
if maxRow<=k, 
    y=[m, 1:maxRow]
else
	% initial value of centroid
    if isRand,
        p = randperm(size(m,1));      % random initialization
        for i=1:k
            c(i,:)=m(p(i),:);  
    	end
    else
        for i=1:k
            c(i,:)=m(i,:);        % sequential initialization
    	end
    end
    
	temp=zeros(maxRow,1);   % initialize as zero vector
	while 1,
    itr = itr+1;
        if isSimilarity,
            d=similarityMatrix(m,c);  % calculate objcets-centroid distances
            [z,g]=max(d,[],2);  % find group matrix g
        else
            d=distMatrix(m,c);  % calculate objcets-centroid distances
            [z,g]=min(d,[],2);  % find group matrix g
        end
        
        if (g==temp),
            break;          % stop the iteration
        else
            temp=g;         % copy group matrix to temporary variable
        end
        for i=1:k
            f=find(g==i);
            if f            % only compute centroid if f is not empty
                c(i,:)=mean(m(find(g==i),:),1);
            end
        end
	end

    exemplars = unique(g)
    c_avg = zeros(size(exemplars,1), size(c,2));
    c_total = zeros(size(exemplars,1), size(c,2));
    size(c_avg)
    for j=1:size(exemplars, 1)
        index = find(g==exemplars(j));
        c_total(j,:) = sum(m(index', :),1,'double')
        c_avg(j,:) = c_total(j,:)./size(index,1);
    end
end

function d=similarityMatrix(A,B)
    d = normprod(A,B'); 

function d=distMatrix(A,B)    
    [hA,wA]=size(A);
    [hB,wB]=size(B);
    if wA ~= wB,  error(' second dimension of A and B must be the same'); end
    for k=1:wA
         C{k}= repmat(A(:,k),1,hB);
         D{k}= repmat(B(:,k),1,hA);
    end
    S=zeros(hA,hB);
    for k=1:wA
         S=S+(C{k}-D{k}').^2;
    end
    d=sqrt(S);