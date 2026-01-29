"""Sensor modeling and event generation."""
from __future__ import annotations

from dataclasses import dataclass
from typing import Iterable, Optional

import numpy as np

from config import Beam, SensorModel


@dataclass
class BeamInterval:
    beam_id: int
    t_enter: float
    t_exit: float


@dataclass
class Event:
    beam_id: int
    edge: str
    time: float


def apply_sensor_model(
    intervals: Iterable[BeamInterval],
    sensor: SensorModel,
    rng: np.random.Generator,
) -> tuple[list[Event], dict[int, tuple[float, float]]]:
    """Apply latency/jitter to ideal intervals and return edge events.

    Returns list of events and dict beam_id -> (t_fall, t_rise).
    """
    events: list[Event] = []
    edge_times: dict[int, tuple[float, float]] = {}

    for interval in intervals:
        if interval.t_exit - interval.t_enter < sensor.min_pulse_width:
            continue
        if rng.random() < sensor.miss_probability:
            continue

        t_fall = interval.t_enter + sensor.fall_latency_mean + rng.normal(0.0, sensor.fall_jitter_std)
        t_rise = interval.t_exit + sensor.rise_latency_mean + rng.normal(0.0, sensor.rise_jitter_std)

        if t_rise < t_fall:
            t_rise = t_fall

        events.append(Event(interval.beam_id, "fall", t_fall))
        events.append(Event(interval.beam_id, "rise", t_rise))
        edge_times[interval.beam_id] = (t_fall, t_rise)

    events.sort(key=lambda e: e.time)
    return events, edge_times


def polling_events(
    beams: Iterable[Beam],
    edge_times: dict[int, tuple[float, float]],
    poll_rate_hz: float,
    t_end: float,
) -> list[Event]:
    """Generate quantized edge events by polling digital outputs."""
    dt = 1.0 / poll_rate_hz
    times = np.arange(0.0, t_end + dt, dt)

    events: list[Event] = []
    for beam in beams:
        fall, rise = edge_times.get(beam.beam_id, (None, None))
        prev_state = 1
        for t in times:
            if fall is not None and rise is not None and fall <= t <= rise:
                state = 0
            else:
                state = 1
            if state != prev_state:
                edge = "fall" if state == 0 else "rise"
                events.append(Event(beam.beam_id, edge, float(t)))
                prev_state = state
    events.sort(key=lambda e: e.time)
    return events


def find_edge_time(events: Iterable[Event], beam_id: int, edge: str) -> Optional[float]:
    for event in events:
        if event.beam_id == beam_id and event.edge == edge:
            return event.time
    return None
