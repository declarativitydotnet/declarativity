function classify(accepted_basename, rejected_basename, cavg_basename, score_basename) 
    hrs = {'06', '12', '18', '24'};
    d = {'01', '02', '03', '04', '05', '06', '07', '08', '09', '10', '11', '12', '13', '14', '15', '16'};
    
    for day=1:size(d,2)
        next = day;
        for hour=1:size(hrs,2)
            h = hour + 1;
            if(hour > 3)
                next = day + 1;
                h = 1;
            end
            
            cName = strcat(cavg_basename, '200703', d(day), hrs(hour), '_', 'cavg')      
            aName = strcat(accepted_basename, '_200703', d(next), '_', hrs(h), '.dat')
            rName = strcat(rejected_basename, '_200703', d(next), '_', hrs(h), '.dat')
            filename = strcat(score_basename, '_200703', d(day), '_', hrs(hour))

            amat = load(char(aName));
            rmat = load(char(rName));
            c_avg = load(char(cName));
            score_graph(c_avg, amat, rmat, filename);
        end
    end
end