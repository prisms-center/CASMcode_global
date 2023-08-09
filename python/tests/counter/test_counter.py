from libcasm.counter import FloatCounter, IntCounter


def test_IntCounter():
    counter = IntCounter(
        initial=[0] * 3,
        final=[2] * 3,
        increment=[2] * 3,
    )
    assert isinstance(counter, IntCounter)
    assert counter.valid() is True
    values = [x for x in counter]
    assert len(values) == 8
    assert counter.valid() is False

    counter.reset()
    assert counter.valid() is True
    values = [x for x in counter]
    assert len(values) == 8


def test_FloatCounter():
    counter = FloatCounter(
        initial=[0.0] * 3,
        final=[2.0] * 3,
        increment=[1.0] * 3,
    )
    assert isinstance(counter, FloatCounter)
    assert counter.valid() is True
    values = [x for x in counter]
    assert len(values) == 27
    assert counter.valid() is False

    counter.reset()
    assert counter.valid() is True
    values = [x for x in counter]
    assert len(values) == 27
