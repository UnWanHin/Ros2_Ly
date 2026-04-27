from glob import glob
from setuptools import find_packages, setup

package_name = "decision_viz"

setup(
    name=package_name,
    version="0.1.0",
    packages=find_packages(exclude=["test"]),
    data_files=[
        ("share/ament_index/resource_index/packages", [f"resource/{package_name}"]),
        (f"share/{package_name}", ["package.xml", "README.md", "requirements.txt"]),
        (f"share/{package_name}/config", glob("config/*.yaml")),
        (f"share/{package_name}/sample", glob("sample/*.jsonl")),
    ],
    install_requires=["setuptools"],
    zip_safe=True,
    maintainer="unwanhin",
    maintainer_email="unwanhin@example.com",
    description="Offline pygame decision trace visualizer for the sentry behavior tree.",
    license="Proprietary",
    tests_require=["pytest"],
    entry_points={
        "console_scripts": [
            "decision-viz = decision_viz.main:main",
            "decision-viz-start = decision_viz.start:main",
            "decision-viz-mock-inputs = decision_viz.mock_inputs:main",
            "decision-viz-ros-topic-monitor = decision_viz.ros_topic_monitor:main",
        ],
    },
)
