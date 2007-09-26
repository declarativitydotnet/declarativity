class curry:
    """From the Python Cookbook"""
    def __init__(self, fun, *args, **kwargs):
        self.fun = fun
        self.pending = args[:]
        self.kwargs = kwargs.copy()

    def __call__(self, *args, **kwargs):
        if kwargs and self.kwargs:
            kw = self.kwargs.copy()
            kw.update(kwargs)
        else:
            kw = kwargs or self.kwargs
        return self.fun(*(self.pending + args), **kw)

def retryapply(method, args, maxtries, sleeptime):
    """Retry idempotent method up to maxtries times until it succeeds"""
    import time
    tries = 0
    for i in range(maxtries):
        try:
            rval = apply(method, args)
        except:
            tries += 1
            time.sleep(sleeptime)
        else:
            break
    if tries == maxtries:
        raise "Could not apply method in %d tries" % maxtries
    return rval
