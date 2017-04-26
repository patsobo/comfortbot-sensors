import rospy
from move_base_msgs.msg import MoveBaseAction, MoveBaseGoal
import actionlib
from actionlib_msgs.msg import *
from geometry_msgs.msg import Pose, Point, Quaternion

class GoToPose():
    def __init__(self):

        self.goal_sent = False

	rospy.on_shutdown(self.shutdown)
	
	self.move_base = actionlib.SimpleActionClient("move_base", MoveBaseAction)
	rospy.loginfo("Wait for the action server to come up")

	self.move_base.wait_for_server(rospy.Duration(5))

    def goto(self, pos, quat):

        # Send a goal
        self.goal_sent = True
	goal = MoveBaseGoal()
	goal.target_pose.header.frame_id = 'map'
	goal.target_pose.header.stamp = rospy.Time.now()
        goal.target_pose.pose = Pose(Point(pos['x'], pos['y'], 0.000),
                                     Quaternion(quat['r1'], quat['r2'], quat['r3'], quat['r4']))

	# Start moving
        self.move_base.send_goal(goal)

	# Allow TurtleBot up to 30 seconds to complete task
	success = self.move_base.wait_for_result(rospy.Duration(30)) 

        state = self.move_base.get_state()
        result = False

        if success and state == GoalStatus.SUCCEEDED:
            result = True
        else:
            self.move_base.cancel_goal()

        self.goal_sent = False
        return result

    def shutdown(self):
        if self.goal_sent:
            self.move_base.cancel_goal()
        rospy.loginfo("Stop")
        rospy.sleep(1)

if __name__ == '__main__':

    x1 = -.272
    x2 = 0.37
    x3 = 0.835
    x4 = 1.37

    y1 = 1.07
    y2 = 0.63
    y3 = 0.30
    y4 = -0.12

    xgoal = x1
    ygoal = y1

    try:
        rospy.init_node('nav_test', anonymous=False)
        navigator = GoToPose()

        # Customize the following values so they are appropriate for your location
        position = {'x': xgoal, 'y' : ygoal}
        quaternion = {'r1' : 0.000, 'r2' : 0.000, 'r3' : 0.000, 'r4' : 1.000}

        rospy.loginfo("Go to (%s, %s) pose", position['x'], position['y'])
        success = navigator.goto(position, quaternion)

        if success:
            rospy.loginfo("Success")
        else:
            rospy.loginfo("Fail")

        # Sleep to give the last log messages time to be sent
        rospy.sleep(1)

        take_measurements(x, y)

    except rospy.ROSInterruptException:
        rospy.loginfo("Ctrl-C caught. Quitting")
    if xgoal == x1:
        xgoal = x2
    if xgoal == x2:
        xgoal = x3
    if xgoal == x3:
        xgoal = x4
    if xgoal == x4 and ygoal == y1:
        xgoal = x1
        ygoal = y2
    if xgoal == x4 and ygoal == y2:
        xgoal = x1
        ygoal = y3
    if xgoal == x4 and ygoal == y3:
        xgoal = x1
        ygoal = y4
    if xgoal == x4 and ygoal == y4:
        break