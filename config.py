"""Configuration dataclasses and CLI parsing helpers for the foosball sensor simulator."""
from __future__ import annotations

from dataclasses import dataclass
from typing import Optional, Sequence


@dataclass
class Beam:
    """Line-segment beam defined by endpoints A->B in meters."""

    beam_id: int
    ax: float
    ay: float
    bx: float
    by: float


@dataclass
class Shot:
    """A single shot's initial state and velocity."""

    x0: float
    y0: float
    vx: float
    vy: float

    @property
    def speed(self) -> float:
        return (self.vx**2 + self.vy**2) ** 0.5

    @property
    def theta(self) -> float:
        import math

        return math.atan2(self.vy, self.vx)


@dataclass
class SensorModel:
    """Sensor latency and jitter parameters in seconds."""

    fall_latency_mean: float
    rise_latency_mean: float
    fall_jitter_std: float
    rise_jitter_std: float
    miss_probability: float = 0.0
    min_pulse_width: float = 0.0


@dataclass
class SimulationConfig:
    goal_width: float
    goal_depth: float
    ball_radius: float
    beam_spacing: float
    beam_x1: float
    beam_x2: float
    beams: Sequence[Beam]
    sensor: SensorModel


@dataclass
class ExperimentConfig:
    n_shots: int
    seed: Optional[int]
    speed_min: float
    speed_max: float
    angle_min: Optional[float]
    angle_max: Optional[float]
    angle_sigma: Optional[float]
    poll_rate_hz: float
    plots: bool
    visualize: bool
    mode: str
    cheat_mode: bool
    ratio_mapping: str
    export_csv: Optional[str]


DEFAULT_GOAL_WIDTH = 0.20
DEFAULT_GOAL_DEPTH = 0.10
DEFAULT_BALL_RADIUS = 0.017
DEFAULT_BEAM_X1 = 0.02
DEFAULT_BEAM_X2 = 0.22


DEFAULT_SENSOR_LATENCY = 0.002
DEFAULT_SENSOR_JITTER = 0.0002


DEFAULT_SPEED_MIN = 8.0
DEFAULT_SPEED_MAX = 25.0
DEFAULT_ANGLE_SIGMA_DEG = 15.0


def build_default_beams(goal_width: float, x1: float, x2: float) -> list[Beam]:
    half_w = goal_width / 2.0
    return [
        Beam(beam_id=1, ax=x1, ay=-half_w, bx=x1, by=half_w),
        Beam(beam_id=2, ax=x2, ay=-half_w, bx=x2, by=half_w),
    ]


def parse_args(argv: Optional[Sequence[str]] = None):
    import argparse
    import math

    parser = argparse.ArgumentParser(description="Foosball IR beam speed sensor simulator")
    parser.add_argument("--mode", choices=["single", "batch"], default="batch")
    parser.add_argument("--n_shots", type=int, default=500)
    parser.add_argument("--seed", type=int, default=None)
    parser.add_argument("--beam_spacing", type=float, default=0.20)
    parser.add_argument("--goal_width", type=float, default=DEFAULT_GOAL_WIDTH)
    parser.add_argument("--goal_depth", type=float, default=DEFAULT_GOAL_DEPTH)
    parser.add_argument("--ball_radius", type=float, default=DEFAULT_BALL_RADIUS)
    parser.add_argument("--speed_min", type=float, default=DEFAULT_SPEED_MIN)
    parser.add_argument("--speed_max", type=float, default=DEFAULT_SPEED_MAX)
    parser.add_argument("--angle_sigma", type=float, default=DEFAULT_ANGLE_SIGMA_DEG)
    parser.add_argument("--angle_min", type=float, default=None)
    parser.add_argument("--angle_max", type=float, default=None)
    parser.add_argument("--sensor_latency_ms", type=float, default=DEFAULT_SENSOR_LATENCY * 1000.0)
    parser.add_argument("--sensor_jitter_ms", type=float, default=DEFAULT_SENSOR_JITTER * 1000.0)
    parser.add_argument("--poll_rate_hz", type=float, default=0.0)
    parser.add_argument("--plots", action="store_true")
    parser.add_argument("--visualize", action="store_true")
    parser.add_argument("--miss_probability", type=float, default=0.0)
    parser.add_argument("--min_pulse_ms", type=float, default=0.0)
    parser.add_argument("--cheat_mode", action="store_true")
    parser.add_argument(
        "--ratio_mapping",
        choices=["default", "zero", "poly"],
        default="default",
        help="Friend estimator ratio->theta mapping",
    )
    parser.add_argument("--poly_coeffs", type=str, default=None)
    parser.add_argument("--export_csv", type=str, default=None)

    args = parser.parse_args(argv)

    angle_sigma = math.radians(args.angle_sigma) if args.angle_sigma is not None else None
    angle_min = math.radians(args.angle_min) if args.angle_min is not None else None
    angle_max = math.radians(args.angle_max) if args.angle_max is not None else None

    sensor = SensorModel(
        fall_latency_mean=args.sensor_latency_ms / 1000.0,
        rise_latency_mean=args.sensor_latency_ms / 1000.0,
        fall_jitter_std=args.sensor_jitter_ms / 1000.0,
        rise_jitter_std=args.sensor_jitter_ms / 1000.0,
        miss_probability=args.miss_probability,
        min_pulse_width=args.min_pulse_ms / 1000.0,
    )

    beam_x1 = DEFAULT_BEAM_X1
    beam_x2 = beam_x1 + args.beam_spacing
    beams = build_default_beams(args.goal_width, beam_x1, beam_x2)

    sim_config = SimulationConfig(
        goal_width=args.goal_width,
        goal_depth=args.goal_depth,
        ball_radius=args.ball_radius,
        beam_spacing=args.beam_spacing,
        beam_x1=beam_x1,
        beam_x2=beam_x2,
        beams=beams,
        sensor=sensor,
    )

    exp_config = ExperimentConfig(
        n_shots=args.n_shots,
        seed=args.seed,
        speed_min=args.speed_min,
        speed_max=args.speed_max,
        angle_min=angle_min,
        angle_max=angle_max,
        angle_sigma=angle_sigma,
        poll_rate_hz=args.poll_rate_hz,
        plots=args.plots,
        visualize=args.visualize,
        mode=args.mode,
        cheat_mode=args.cheat_mode,
        ratio_mapping=args.ratio_mapping,
        export_csv=args.export_csv,
    )

    poly_coeffs = None
    if args.poly_coeffs:
        poly_coeffs = [float(x) for x in args.poly_coeffs.split(",")]

    return sim_config, exp_config, poly_coeffs
