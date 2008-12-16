function plotCAscore(ctr, varargin);
    for i=1:length(varargin)
        a = varargin{i};
        n = size(a,1);
        score = cScore(ctr,a);
        args{1,i} = sort(score);
        args{2,i} = (1:n)/n;
    end
    semilogx(args{:});
    L = legend('Accepted', 'Rejected', 'Location', 'NorthEast');
    set(L, 'FontSize', 16)
end

function score = cScore(ctr, a)
    score= normprod(a, ctr');
    score=max(score,[],2);
end

