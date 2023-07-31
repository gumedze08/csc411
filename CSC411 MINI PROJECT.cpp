// ConsoleApplication111111111.cpp : This file contains the 'main' function. Program execution begins and ends there.
//



#include <iostream>
#include <fstream>
#include <thread>
#include <queue>
#include <mutex>
#include <condition_variable>
#include <random>
#include <string>
#include <unordered_map>


class ITstudent {
public:
	ITstudent(const std::string& name, const std::string& id, const std::string& programme,
		const std::unordered_map<std::string, int>& courses)
		: name(name), id(id), programme(programme), courses(courses) {}

	double calculate_average_mark() {
		double total_marks = 0;
		for (const auto& course : courses) {
			total_marks += course.second;
		}
		return total_marks / courses.size();
	}

	bool is_passed() {
		double average_mark = calculate_average_mark();
		return average_mark >= 50;
	}

	std::string name;
	std::string id;
	std::string programme;
	std::unordered_map<std::string, int> courses;
};

class Buffer {
public:
	Buffer(size_t max_size) : max_size(max_size) {}

	void add(int value) {
		std::unique_lock<std::mutex> lock(mutex_);
		not_full_condition.wait(lock, [this] {
			return buffer_.size() < max_size;
			});
		buffer_.push(value);
		not_empty_condition.notify_one();
	}

	int remove() {
		std::unique_lock<std::mutex> lock(mutex_);
		not_empty_condition.wait(lock, [this] {
			return !buffer_.empty();
			});
		int value = buffer_.front();
		buffer_.pop();
		not_full_condition.notify_one();
		return value;
	}

private:
	std::queue<int> buffer_;
	size_t max_size;
	std::mutex mutex_;
	std::condition_variable not_full_condition;
	std::condition_variable not_empty_condition;
};

ITstudent generate_student_data() {
	static std::random_device rd;
	static std::mt19937 gen(rd());
	static std::uniform_int_distribution<int> id_dist(10000000, 99999999);
	static std::uniform_int_distribution<int> mark_dist(0, 100);
	static std::unordered_map<int, std::string> programmes = {
		{0, "Computer Science"},
		{1, "Software Engineering"},
		{2, "Information Technology"}
	};
	static std::vector<std::string> courses = {
		"Computer Networking",
		"Computer Architecture",
		"Database Management"
	};

	std::string name = "Student " + std::to_string(gen());
	std::string student_id = std::to_string(id_dist(gen));
	std::string programme = programmes[gen() % 3];
	std::unordered_map<std::string, int> course_marks;
	for (const auto& course : courses) {
		course_marks[course] = mark_dist(gen);
	}

	return ITstudent(name, student_id, programme, course_marks);
}

void save_student_data_as_xml(const ITstudent& student, int file_number) {
	std::string file_name = "student" + std::to_string(file_number) + ".xml";

	std::ofstream xml_file(file_name);
	xml_file << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
	xml_file << "<Student>\n";
	xml_file << "    <Name>" << student.name << "</Name>\n";
	xml_file << "    <StudentID>" << student.id << "</StudentID>\n";
	xml_file << "    <Programme>" << student.programme << "</Programme>\n";
	xml_file << "    <Courses>\n";
	for (const auto& course : student.courses) {
		xml_file << "        <Course>\n";
		xml_file << "            <Name>" << course.first << "</Name>\n";
		xml_file << "            <Mark>" << course.second << "</Mark>\n";
		xml_file << "        </Course>\n";
	}
	xml_file << "    </Courses>\n";
	xml_file << "</Student>\n";
	xml_file.close();
}

ITstudent read_student_data_from_xml(int file_number) {
	std::string file_name = "student" + std::to_string(file_number) + ".xml";

	std::ifstream xml_file(file_name);
	std::string line;
	std::string xml_contents;
	while (std::getline(xml_file, line)) {
		xml_contents += line;
	}
	xml_file.close();

	// Parse XML and gather student information
	// ...

	return ITstudent("Student Name", "12345678", "Programme",
		{ {"Course 1", 80}, {"Course 2", 75}, {"Course 3", 90} });
}

void producer(std::shared_ptr<Buffer> buffer) {
	for (int i = 1; i <= 10; ++i) {
		ITstudent student = generate_student_data();
		save_student_data_as_xml(student, i);

		buffer->add(i);
	}
}

void consumer(std::shared_ptr<Buffer> buffer) {
	while (true) {
		int file_number = buffer->remove();

		ITstudent student = read_student_data_from_xml(file_number);

		double average_mark = student.calculate_average_mark();
		std::string pass_status = student.is_passed() ? "Pass" : "Fail";

		std::cout << "==== Student Information ====\n";
		std::cout << "Name: " << student.name << "\n";
		std::cout << "Student ID: " << student.id << "\n";
		std::cout << "Programme: " << student.programme << "\n";
		std::cout << "Courses and Marks:\n";
		for (const auto& course : student.courses) {
			std::cout << course.first << ": " << course.second << "\n";
		}
		std::cout << "Average Mark: " << average_mark << "\n";
		std::cout << "Pass/Fail: " << pass_status << "\n";
		std::cout << "=============================\n\n";

		// Removing the XML file
		std::string file_name = "student" + std::to_string(file_number) + ".xml";
		std::remove(file_name.c_str());
	}
}

int main() {
	std::shared_ptr<Buffer> buffer = std::make_shared<Buffer>(10);

	std::thread producer_thread(producer, buffer);
	std::thread consumer_thread(consumer, buffer);

	producer_thread.join();
	consumer_thread.join();

	return 0;
}



//int main()
//{
//    std::cout << "Hello World!\n"; 
//}

// Run program: Ctrl + F5 or Debug > Start Without Debugging menu
// Debug program: F5 or Debug > Start Debugging menu

// Tips for Getting Started: 
//   1. Use the Solution Explorer window to add/manage files
//   2. Use the Team Explorer window to connect to source control
//   3. Use the Output window to see build output and other messages
//   4. Use the Error List window to view errors
//   5. Go to Project > Add New Item to create new code files, or Project > Add Existing Item to add existing code files to the project
//   6. In the future, to open this project again, go to File > Open > Project and select the .sln file
