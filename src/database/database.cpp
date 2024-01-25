#include "database/database.h"

#include "third_party/sqlite/sqlite_orm.h"


using namespace sqlite_orm;

namespace buried {
	inline auto InitStorage(const std::string& path) {
		return make_storage(
			path,make_table("buried_data",
							make_column("id",&BuriedDb::Data::id,primary_key().autoincrement()),
							make_column("priority",&BuriedDb::Data::priority),
							make_column("timestamp",&BuriedDb::Data::timestamp),
							make_column("content",&BuriedDb::Data::content)
							)
		);
		
	}

	class BuriedDbImpl {
	private:
		using DBStorage = decltype(InitStorage(""));
	private:
		std::string m_db_path;
		std::unique_ptr<DBStorage> m_storage;
	public:
		BuriedDbImpl(std::string db_path) :m_db_path(db_path) {
			m_storage = std::make_unique<DBStorage>(InitStorage(m_db_path));
			m_storage->sync_schema();
		}

		~BuriedDbImpl() {}

		void InsertData(const BuriedDb::Data& data) {
			auto guard = m_storage->transaction_guard();
			m_storage->insert(data);
			guard.commit();
		}

		void DeleteData(const BuriedDb::Data& data) {
			auto guard = m_storage->transaction_guard();
			m_storage->remove_all<BuriedDb::Data>(
				where(c(&BuriedDb::Data::id) == data.id)
				);
			guard.commit();
		}
		void DeleteDatas(const std::vector<BuriedDb::Data>& datas) {
			auto guard = m_storage->transaction_guard();
			for (const auto& data : datas) {
				m_storage->remove_all<BuriedDb::Data>(
					where(c(&BuriedDb::Data::id) == data.id));
			}
			guard.commit();
		}
		std::vector<BuriedDb::Data> QueryData(int32_t limit_size) {
			auto limited = m_storage->get_all<BuriedDb::Data>(
				order_by(&BuriedDb::Data::priority).desc(), limit(limit_size));
			return limited;
		}
	};

	BuriedDb::BuriedDb(std::string db_path) : m_impl{ std::make_unique<BuriedDbImpl>(db_path) } {}
	BuriedDb::~BuriedDb() {}

	void BuriedDb::InsertData(const Data& data) { m_impl->InsertData(data); }
	void BuriedDb::DeleteData(const Data& data) { m_impl->DeleteData(data); }

	void BuriedDb::DeleteDatas(const std::vector<Data>& datas) {
		m_impl->DeleteDatas(datas);
	}

	std::vector<BuriedDb::Data> BuriedDb::QueryData(int32_t limit) {
		return m_impl->QueryData(limit);
	}
}